// ============================================================
//  AsyncDevice.cpp  –  Cross-platform async LoRa wrapper
// ============================================================

#include "AsyncDevice.h"

// ── Static instance pointer (singleton trampoline) ───────────
AsyncDevice* AsyncDevice::_instance = nullptr;

// ════════════════════════════════════════════════════════════
//  Constructor
// ════════════════════════════════════════════════════════════
AsyncDevice::AsyncDevice(int* error)
	: _rxArmed(false)
	, _head(0)
	, _tail(0)
	, _count(0)
{
	#if defined(ARDUINO)
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

	// Set the error variable to zero in case everything went well
	if(error != nullptr) *error = 0;
}

// ════════════════════════════════════════════════════════════
//  Destructor
// ════════════════════════════════════════════════════════════
AsyncDevice::~AsyncDevice() {
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

	//log_print("[SX1262] Initializing...", "Init Radio...");
	int state = radio->begin(868.0);
	if(state != RADIOLIB_ERR_NONE) {
		// if(log_level == LOG_TERMINAL) log_printf(" Initialization Failed!\nError code: %d\n", state);
		// else log_printf("!\nError code: %d\n", state);
		return nullptr;
	}
	state = radio->setCRC(2);
	if(state != RADIOLIB_ERR_NONE) {
		// if(log_level == LOG_TERMINAL) log_printf(" CRC Initialization Failed!\nError code: %d\n", state);
		// else log_printf("!\nCRC Error: %d\n", state);
		return nullptr;
	}
	// log_print(" Initialization successful!\n", "OK.\n");
	return radio;
}
#else
SX1262* AsyncDevice::init_radio() {
	// now we can create the radio module
	SX1262* radio = new SX1262(new Module(hal, 21, 16, 18, 20 /*The BUSY pin of the module MUST be specified, otherwise error -2 is thrown*/));
	
	//log_print("[SX1262] Initializing...", "Init Radio...");
	/* The module is being initialized with all the default begin() settings
	 * The only settings changed are the following:
	 * - The frequency, according to the EU868 standard must be 868MHz, the default frequency is 434MHz
	 * - The TCXO voltage, which is the crystal which is powering the clock, since this module is not using TCXO, 
	 *   not specifying that will cause error -707 to be thrown, so we need to specify its voltage to be 0
	 */
	int state = radio->begin(868.0 /*EU868 frequency*/, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 0 /*This is not the default value*/, false);
	if(state != RADIOLIB_ERR_NONE) {
		//if(log_level == LOG_TERMINAL) log_printf(" Initialization Failed!\nError code: %d\n", state);
		//else log_printf("!\nError code: %d\n", state);
		return nullptr;
	}
	//log_print(" Initialization successful!\n", "OK.\n");
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
	// 1. Disarm RX so we don't race with an incoming packet during TX
	if (_rxArmed) {
		_radio->standby();
		_rxArmed = false;
	}

	// 2. Transmit (RadioLib blocking call)
	const uint8_t* buf = reinterpret_cast<const uint8_t*>(payload.c_str());
	size_t         len = static_cast<size_t>(payload.length());

	int status = _radio->transmit(buf, len);

	// 3. Always re-arm RX regardless of TX outcome
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
		return false;   // nothing in buffer
	}

	out = str(reinterpret_cast<const char*>(pkt.data));
	// Trim to actual length in case data contains embedded nulls
	out = str(reinterpret_cast<const char*>(pkt.data)).substring(0, pkt.len);

	return true;
}


// ════════════════════════════════════════════════════════════
//  stop()  –  place radio in STANDBY
// ════════════════════════════════════════════════════════════
void AsyncDevice::stop()
{
	_radio->standby();
	_rxArmed = false;
}


// ════════════════════════════════════════════════════════════
//  startRx()  –  arm continuous reception
// ════════════════════════════════════════════════════════════
void AsyncDevice::startRx()
{
	// startReceive() puts the chip in continuous RX;
	// DIO1 fires when a packet CRC passes.
	int err = _radio->startReceive();
	_rxArmed = (err == RADIOLIB_ERR_NONE);
}


// ════════════════════════════════════════════════════════════
//  onReceive()  –  called from DIO1 ISR context
// ════════════════════════════════════════════════════════════
void AsyncDevice::onReceive()
{
	// Read raw bytes from the radio FIFO
	uint8_t  buf[ASYNC_MTU];
	size_t   len = ASYNC_MTU;

	int status = _radio->readData(buf, len);
	if (status != RADIOLIB_ERR_NONE) {
		// Bad packet / CRC error – discard and re-arm
		startRx();
		return;
	}

	// Clamp to MTU (RadioLib should never exceed it, but just in case)
	if (len > ASYNC_MTU) len = ASYNC_MTU;

	// Push into FIFO (drops oldest if full – configurable policy)
	queuePush(buf, static_cast<uint8_t>(len));

	// Re-arm reception immediately so we don't miss the next packet
	startRx();
}


// ════════════════════════════════════════════════════════════
//  Static ISR trampoline
// ════════════════════════════════════════════════════════════
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

// Push a packet produced by the ISR (producer side).
// If the queue is full the OLDEST packet is evicted (drop-tail policy).
bool AsyncDevice::queuePush(const uint8_t* data, uint8_t len)
{
#if defined(ARDUINO)
	ASYNC_ENTER_CRITICAL();
#else
	std::lock_guard<std::mutex> lock(_mutex);
#endif

	if (_count == ASYNC_QUEUE_DEPTH) {
		// Drop oldest: advance head
		_head = (_head + 1) % ASYNC_QUEUE_DEPTH;
		--_count;
	}

	// Write at tail
	memcpy(_queue[_tail].data, data, len);
	_queue[_tail].len = len;
	_tail = (_tail + 1) % ASYNC_QUEUE_DEPTH;
	++_count;

#if defined(ARDUINO)
	ASYNC_EXIT_CRITICAL();
#endif

	return true;
}

// Pop the oldest packet (consumer side).
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
