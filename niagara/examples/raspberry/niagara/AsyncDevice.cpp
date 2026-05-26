// ============================================================
//  AsyncDevice.cpp  –  Cross-platform async LoRa wrapper
// ============================================================

#include "AsyncDevice.h"

// ── Platform-specific SPI Mutex Macros ───────────────────────
#if defined(ARDUINO) && defined(ESP32)
	portMUX_TYPE async_queue_mux = portMUX_INITIALIZER_UNLOCKED;
	#define SPI_LOCK()   xSemaphoreTake(this->_spiMutex, portMAX_DELAY)
	#define SPI_UNLOCK() xSemaphoreGive(this->_spiMutex)
#elif !defined(ARDUINO)
	#define SPI_LOCK()   this->_linuxSpiMutex.lock()
	#define SPI_UNLOCK() this->_linuxSpiMutex.unlock()
#else
	#define SPI_LOCK()   ((void)0)
	#define SPI_UNLOCK() ((void)0)
#endif

AsyncDevice* AsyncDevice::_instance = nullptr;

// ════════════════════════════════════════════════════════════
//  Constructor
// ════════════════════════════════════════════════════════════
AsyncDevice::AsyncDevice(int* error)
	: _rxArmed(false)
	, _txActive(false)
	, _packetReady(false)
	, _head(0)
	, _tail(0)
	, _count(0)
{
	#if defined(ARDUINO)
		#if defined(ESP32)
		_processTaskHandle = nullptr;
		_spiMutex = xSemaphoreCreateMutex();
		#endif
	#else
	AsyncDevice::hal = new PiHal(1);
	_processThread = nullptr;
	_threadRunning = false;
	#endif

	SX1262* radio_return = init_radio();
	if(radio_return == nullptr) {
		if(error != nullptr) *error = -1;
		return;
	}
	AsyncDevice::_radio = radio_return;
	_instance = this;

	_radio->setDio1Action(_isrTrampoline);
	startRx();

	#if defined(ARDUINO) && defined(ESP32)
	xTaskCreatePinnedToCore(
		_taskTrampoline, "AsyncLoRaTask", 4096, this, 1, &_processTaskHandle, 0
	);
	#elif !defined(ARDUINO)
	// Launch std::thread for Raspberry Pi background processing
	_threadRunning = true;
	_processThread = new std::thread(_threadTrampoline, this);
	#endif

	if(error != nullptr) *error = 0;
}

// ════════════════════════════════════════════════════════════
//  Destructor
// ════════════════════════════════════════════════════════════
AsyncDevice::~AsyncDevice() {
	#if defined(ARDUINO) && defined(ESP32)
	if (_processTaskHandle) {
		vTaskDelete(_processTaskHandle);
		_processTaskHandle = nullptr;
	}
	if (_spiMutex) {
		vSemaphoreDelete(_spiMutex);
		_spiMutex = nullptr;
	}
	#elif !defined(ARDUINO)
	_threadRunning = false;
	if (_processThread) {
		if (_processThread->joinable()) {
			_processThread->join();
		}
		delete _processThread;
		_processThread = nullptr;
	}
	#endif

	if(AsyncDevice::_radio) {
		delete AsyncDevice::_radio;
		AsyncDevice::_radio = nullptr;
	}
	
	#ifndef ARDUINO
	if(AsyncDevice::hal) {
		delete AsyncDevice::hal;
		AsyncDevice::hal = nullptr;
	}
	#endif
}

#if defined(ARDUINO)
SX1262* AsyncDevice::init_radio() {
	// SX1262 has the following connections on this board:
	// NSS pin:   8
	// DIO1 pin:  14
	// NRST pin:  12
	// BUSY pin:  13
	SX1262* radio = new SX1262(new Module(8, 14, 12, 13));
	int state = radio->begin(868.0);
	if(state != RADIOLIB_ERR_NONE) return nullptr;
	state = radio->setCRC(2);
	if(state != RADIOLIB_ERR_NONE) return nullptr;
	return radio;
}
#else
SX1262* AsyncDevice::init_radio() {
	SX1262* radio = new SX1262(new Module(hal, 21, 16, 18, 20 /*The BUSY pin of the module MUST be specified, otherwise error -2 is thrown*/));
	/* The module is being initialized with all the default begin() settings
	 * The only settings changed are the following:
	 * - The frequency, according to the EU868 standard must be 868MHz, the default frequency is 434MHz
	 * - The TCXO voltage, which is the crystal which is powering the clock, since this module is not using TCXO, 
	 *   not specifying that will cause error -707 to be thrown, so we need to specify its voltage to be 0
	 */
	int state = radio->begin(868.0 /*EU868 frequency*/, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 0 /*This is not the default value*/, false);
	if(state != RADIOLIB_ERR_NONE) return nullptr;
	return radio;
}
#endif

size_t AsyncDevice::getMTU() {
	return _radio->maxPacketLength;
}

int AsyncDevice::send(const str& payload)
{
	SPI_LOCK();
	_txActive = true;
	if (_rxArmed) {
		_radio->standby();
		_rxArmed = false;
	}

	const uint8_t* buf = reinterpret_cast<const uint8_t*>(payload.c_str());
	size_t         len = static_cast<size_t>(payload.length());

	int status = _radio->transmit(buf, len);

	_txActive = false;
	SPI_UNLOCK();
	startRx();
	return status;
}

bool AsyncDevice::recv(str& out)
{
	AsyncPacket pkt;
	if (!queuePop(pkt)) return false;

	out = str((char*)pkt.data).substring(0, pkt.len);
	return true;
}

void AsyncDevice::stop()
{
	SPI_LOCK();
	_radio->standby();
	_rxArmed = false;
	SPI_UNLOCK();
}

int AsyncDevice::startRx()
{
	SPI_LOCK();
	int err = _radio->startReceive();
	_rxArmed = (err == RADIOLIB_ERR_NONE);
	SPI_UNLOCK();
	return (err == RADIOLIB_ERR_NONE) ? 0 : err;
}

void AsyncDevice::onReceive()
{
	if (!_rxArmed || _txActive) return;
	_packetReady = true;
}

void AsyncDevice::process()
{
	if (_packetReady) {
		_packetReady = false;

		SPI_LOCK();
		uint8_t  buf[ASYNC_MTU];
		size_t   len = _radio->getPacketLength();

		int status = _radio->readData(buf, len);
		SPI_UNLOCK();

		if (status != RADIOLIB_ERR_NONE) {
			startRx();
			return;
		}

		if (len > ASYNC_MTU) len = ASYNC_MTU;
		queuePush(buf, static_cast<uint8_t>(len));
		startRx();
	}
}

// ════════════════════════════════════════════════════════════
//  Background Thread/Task Trampolines
// ════════════════════════════════════════════════════════════
#if defined(ARDUINO) && defined(ESP32)
void AsyncDevice::_taskTrampoline(void* pvParameters)
{
	AsyncDevice* instance = static_cast<AsyncDevice*>(pvParameters);
	for(;;) {
		instance->process();
		vTaskDelay(pdMS_TO_TICKS(1)); 
	}
}
#elif !defined(ARDUINO)
void AsyncDevice::_threadTrampoline(AsyncDevice* instance)
{
	while (instance->_threadRunning) {
		instance->process();
		// Yield/sleep slightly to prevent 100% CPU usage on the Pi
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
#endif

#if defined(ARDUINO)
void IRAM_ATTR AsyncDevice::_isrTrampoline()
#else
void AsyncDevice::_isrTrampoline()
#endif
{
	if (_instance) _instance->onReceive();
}

// ════════════════════════════════════════════════════════════
//  Ring-buffer helpers
// ════════════════════════════════════════════════════════════
bool AsyncDevice::queuePush(const uint8_t* data, uint8_t len)
{
#if defined(ARDUINO)
	ASYNC_ENTER_CRITICAL();
#else
	std::lock_guard<std::mutex> lock(_queueMutex);
#endif

	if (_count == ASYNC_QUEUE_DEPTH) {
		_head = (_head + 1) % ASYNC_QUEUE_DEPTH;
		--_count;
	}

	memcpy(_queue[_tail].data, data, len);
	_queue[_tail].len = len;
	_tail = (_tail + 1) % ASYNC_QUEUE_DEPTH;
	++_count;

#if defined(ARDUINO)
	ASYNC_EXIT_CRITICAL();
#endif
	return true;
}

bool AsyncDevice::queuePop(AsyncPacket& out)
{
#if defined(ARDUINO)
	ASYNC_ENTER_CRITICAL();
#else
	std::lock_guard<std::mutex> lock(_queueMutex);
#endif

	if (_count == 0) {
#if defined(ARDUINO)
		ASYNC_EXIT_CRITICAL();
#endif
		return false;
	}

	out   = _queue[_head];
	_head = (_head + 1) % ASYNC_QUEUE_DEPTH;
	--_count;

#if defined(ARDUINO)
	ASYNC_EXIT_CRITICAL();
#endif
	return true;
}

uint8_t AsyncDevice::queueCount() const
{
	return _count;
}