#ifndef ASYNC_DEVICE_H
#define ASYNC_DEVICE_H

// ============================================================
//  AsyncDevice.h  –  Cross-platform async LoRa wrapper
//  Tested with RadioLib SX126x on Arduino & Raspberry Pi
// ============================================================

#if defined(ARDUINO)
	#include <Arduino.h>
	#include <RadioLib.h>
	using str = String;
#else
	// Raspberry Pi (Linux) – assume RadioLib installed system-wide
	#include <RadioLib/RadioLib.h>
	// include the hardware abstraction layer
	#include "hal/RPi/PiHal.h"
	#include <string>
	#include <cstring>
	using str = std::string;
#endif

#include <stdint.h>

// ── Buffer configuration ────────────────────────────────────
static constexpr uint8_t  ASYNC_MTU          = 255;  // max bytes per LoRa packet
static constexpr uint8_t  ASYNC_QUEUE_DEPTH  = 8;    // max packets kept in FIFO

// ── Platform-specific ISR / atomic helpers ──────────────────
#if defined(ARDUINO)
	#define ASYNC_ENTER_CRITICAL()  noInterrupts()
	#define ASYNC_EXIT_CRITICAL()   interrupts()
	#define ASYNC_VOLATILE          volatile
#else
	#include <atomic>
	#include <mutex>
	// On Linux/RPi we use a mutex instead of disabling interrupts
	#define ASYNC_VOLATILE
#endif


// ──────────────────────────────────────────────────────────────
//  Packet type stored in the FIFO ring-buffer
// ──────────────────────────────────────────────────────────────
struct AsyncPacket {
		uint8_t  data[ASYNC_MTU];
		uint8_t  len;
};


// ──────────────────────────────────────────────────────────────
//  AsyncDevice
// ──────────────────────────────────────────────────────────────
class AsyncDevice {
public:
		// ── Constructor ───────────────────────────────────────────
		explicit AsyncDevice(SX126x* radio);

		// ── Public API ────────────────────────────────────────────

		/**
		 * Blocking transmit.
		 * Pauses RX, sends the packet, then immediately re-arms RX.
		 * Returns a RadioLib status code (RADIOLIB_ERR_NONE == 0 on success).
		 */
		int send(const str& payload);

		/**
		 * Non-blocking receive.
		 * Pops the oldest packet from the FIFO into `out` and returns true.
		 * Returns false (and leaves `out` unchanged) if the buffer is empty.
		 */
		bool recv(str& out);

		/**
		 * Put the radio into STANDBY – stops background reception.
		 * Call send() or explicitly restart RX with startRx() to wake up.
		 */
		void stop();

		/**
		 * Re-arm continuous reception (called automatically after send/ISR,
		 * but exposed publicly so the caller can restart after stop()).
		 */
		void startRx();

		// ── ISR / callback (must be public so the static trampoline can call it) ─
		void onReceive();   // called from DIO1 interrupt

private:
		// ── Radio handle ─────────────────────────────────────────
		SX126x* _radio;

		// ── State flag ───────────────────────────────────────────
		ASYNC_VOLATILE bool _rxArmed;   // true when radio is in RX mode

		// ── FIFO ring-buffer ─────────────────────────────────────
		AsyncPacket _queue[ASYNC_QUEUE_DEPTH];
		ASYNC_VOLATILE uint8_t _head;   // next slot to read  (consumer)
		ASYNC_VOLATILE uint8_t _tail;   // next slot to write (producer / ISR)
		ASYNC_VOLATILE uint8_t _count;  // packets currently stored

#if !defined(ARDUINO)
		std::mutex _mutex;  // protects the ring-buffer on Linux/RPi
#endif

		// ── Helpers ───────────────────────────────────────────────
		bool     queuePush(const uint8_t* data, uint8_t len);
		bool     queuePop(AsyncPacket& out);
		uint8_t  queueCount() const;

		// ── Static trampoline for RadioLib callback ───────────────
		// RadioLib needs a plain function pointer (or IRAM_ATTR on ESP32).
		// We store a pointer to the single live instance here.
		static AsyncDevice* _instance;

#if defined(ARDUINO)
		static void IRAM_ATTR _isrTrampoline();
#else
		static void           _isrTrampoline();
#endif
};

#endif