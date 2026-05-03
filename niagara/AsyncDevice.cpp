// ============================================================
//  AsyncDevice.cpp  –  Cross-platform async LoRa wrapper
// ============================================================

#include "AsyncDevice.h"

// ── ESP32 Hardware Spinlock & SPI Mutex Macros ───────────────
#if defined(ARDUINO) && defined(ESP32)
	portMUX_TYPE async_queue_mux = portMUX_INITIALIZER_UNLOCKED;
	#define SPI_LOCK()   xSemaphoreTake(_spiMutex, portMAX_DELAY)
	#define SPI_UNLOCK() xSemaphoreGive(_spiMutex)
#else
	#define SPI_LOCK()   ((void)0)
	#define SPI_UNLOCK() ((void)0)
#endif

// ── Static instance pointer (singleton trampoline) ───────────
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
	// use SPI channel 1, because on Waveshare LoRaWAN Hat,
	// the SX1261 CS is connected to CE1
	AsyncDevice::hal = new PiHal(1);
	#endif

	//Initialize the radio module
	SX1262* radio_return = init_radio();
	if(radio_return == nullptr) {
		if(error != nullptr) *error = -1;
		return;
	}
	AsyncDevice::_radio = radio_return;

	// Register this instance as the singleton target for the ISR
	_instance = this;

	// Hook DIO1 → our trampoline
	_radio->setDio1Action(_isrTrampoline);

	// Immediately arm RX
	startRx();

	#if defined(ARDUINO) && defined(ESP32)
	// Launch the background task on Core 0 (Arduino loop runs on Core 1)
	xTaskCreatePinnedToCore(
		_taskTrampoline,      // Task function
		"AsyncLoRaTask",      // Name
		4096,                 // Stack size
		this,                 // Pass this instance to the static trampoline
		1,                    // Priority
		&_processTaskHandle,  // Handle
		0                     // Pin to core 0
	);
	#endif

	// Set the error variable to zero in case everything went well
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
	SX1262* radio = new SX1262(new Module(8, 14, 12, 13));

	int state = radio->begin(868.0);
	if(state != RADIOLIB_ERR_NONE) {
		return nullptr;
	}
	state = radio->setCRC(2);
	if(state != RADIOLIB_ERR_NONE) {
		return nullptr;
	}
	return radio;
}
#else
SX1262* AsyncDevice::init_radio() {
	SX1262* radio = new SX1262(new Module(hal, 21, 16, 18, 20));
	int state = radio->begin(868.0, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 0, false);
	if(state != RADIOLIB_ERR_NONE) {
		return nullptr;
	}
	return radio;
}
#endif

size_t AsyncDevice::getMTU() {
	return _radio->maxPacketLength;
}

// ════════════════════════════════════════════════════════════
//  send()  –  blocking TX, then re-arm RX
// ════════════════════════════════════════════════════════════
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

	// startRx internally locks the mutex, so we call it after unlocking
	startRx();

	return status;
}

// ════════════════════════════════════════════════════════════
//  recv()  –  pop oldest packet from FIFO (non-blocking)
// ════════════════════════════════════════════════════════════
bool AsyncDevice::recv(str& out)
{
	AsyncPacket pkt;
	if (!queuePop(pkt)) {
		return false;
	}

	out = str(reinterpret_cast<const char*>(pkt.data));
	out = str(reinterpret_cast<const char*>(pkt.data)).substring(0, pkt.len);

	return true;
}

// ════════════════════════════════════════════════════════════
//  stop()  –  place radio in STANDBY
// ════════════════════════════════════════════════════════════
void AsyncDevice::stop()
{
	SPI_LOCK();
	_radio->standby();
	_rxArmed = false;
	SPI_UNLOCK();
}

// ════════════════════════════════════════════════════════════
//  startRx()  –  arm continuous reception
// ════════════════════════════════════════════════════════════
void AsyncDevice::startRx()
{
	SPI_LOCK();
	int err = _radio->startReceive();
	_rxArmed = (err == RADIOLIB_ERR_NONE);
	SPI_UNLOCK();
}

// ════════════════════════════════════════════════════════════
//  onReceive()  –  called from DIO1 ISR context
// ════════════════════════════════════════════════════════════
void AsyncDevice::onReceive()
{
	if (!_rxArmed || _txActive) return;
	_packetReady = true;
}

// ════════════════════════════════════════════════════════════
//  process()  –  Deferred ISR processing (Runs automatically on ESP32)
// ════════════════════════════════════════════════════════════
void AsyncDevice::process()
{
	if (_packetReady) {
		_packetReady = false;

		SPI_LOCK();
		uint8_t  buf[ASYNC_MTU];
		size_t   len = _radio->getPacketLength();

		int status = _radio->readData(buf, len);
		SPI_UNLOCK(); // Unlock before calling startRx

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
//  Static Task & ISR Trampolines
// ════════════════════════════════════════════════════════════
#if defined(ARDUINO) && defined(ESP32)
void AsyncDevice::_taskTrampoline(void* pvParameters)
{
	AsyncDevice* instance = static_cast<AsyncDevice*>(pvParameters);
	for(;;) {
		instance->process();
		// A 1 tick (~1ms) delay is strictly required here. It keeps the core
		// from running at 100% capacity and prevents the FreeRTOS Task 
		// Watchdog Timer (TWDT) from crashing the ESP32.
		vTaskDelay(pdMS_TO_TICKS(1)); 
	}
}
#endif

#if defined(ARDUINO)
void IRAM_ATTR AsyncDevice::_isrTrampoline()
#else
void AsyncDevice::_isrTrampoline()
#endif
{
	if (_instance) {
		_instance->onReceive();
	}
}

// ════════════════════════════════════════════════════════════
//  Ring-buffer helpers
// ════════════════════════════════════════════════════════════
bool AsyncDevice::queuePush(const uint8_t* data, uint8_t len)
{
#if defined(ARDUINO)
	ASYNC_ENTER_CRITICAL();
#else
	std::lock_guard<std::mutex> lock(_mutex);
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
	std::lock_guard<std::mutex> lock(_mutex);
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