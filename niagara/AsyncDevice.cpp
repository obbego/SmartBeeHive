// ============================================================
//  AsyncDevice.cpp  –  Cross-platform async LoRa wrapper
// ============================================================

#include "AsyncDevice.h"

// ── Static instance pointer (singleton trampoline) ───────────
AsyncDevice* AsyncDevice::_instance = nullptr;

// ════════════════════════════════════════════════════════════
//  Constructor
// ════════════════════════════════════════════════════════════
AsyncDevice::AsyncDevice(SX126x* radio)
	: _radio(radio)
	, _rxArmed(false)
	, _head(0)
	, _tail(0)
	, _count(0)
{
	// Register this instance as the singleton target for the ISR
	_instance = this;

	// Hook DIO1 → our trampoline
	_radio->setDio1Action(_isrTrampoline);

	// Immediately arm RX
	startRx();
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
#if defined(ARDUINO)
	const uint8_t* buf = reinterpret_cast<const uint8_t*>(payload.c_str());
	size_t         len = static_cast<size_t>(payload.length());
#else
	const uint8_t* buf = reinterpret_cast<const uint8_t*>(payload.c_str());
	size_t         len = payload.size();
#endif

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

#if defined(ARDUINO)
	out = String(reinterpret_cast<const char*>(pkt.data));
	// Trim to actual length in case data contains embedded nulls
	out = String(reinterpret_cast<const char*>(pkt.data)).substring(0, pkt.len);
#else
	out.assign(reinterpret_cast<const char*>(pkt.data),
			   static_cast<size_t>(pkt.len));
#endif

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