#ifndef ASYNC_DEVICE_H
#define ASYNC_DEVICE_H

// ============================================================
//  AsyncDevice.h  –  Cross-platform async LoRa wrapper
//  Tested with RadioLib SX126x on Arduino & Raspberry Pi
// ============================================================

#if defined(ARDUINO)
	#include <Arduino.h>
	#include <RadioLib.h>
	#if defined(ESP32)
		#include <freertos/FreeRTOS.h>
		#include <freertos/task.h>
		#include <freertos/semphr.h>
	#endif
#else
	// Raspberry Pi (Linux) – assume RadioLib installed system-wide
	#include <RadioLib/RadioLib.h>
	// include the hardware abstraction layer
	#include "hal/RPi/PiHal.h"
	#include <string>
	#include <cstring>
#endif

#include <stdint.h>
#include "str.h"

// ── Buffer configuration ────────────────────────────────────
static constexpr uint8_t  ASYNC_MTU          = 255;  // max bytes per LoRa packet
static constexpr uint8_t  ASYNC_QUEUE_DEPTH  = 8;    // max packets kept in FIFO

// ── Platform-specific ISR / atomic helpers ──────────────────
#if defined(ARDUINO)
	#if defined(ESP32)
		// ESP32 dual-core requires a spinlock to protect variables across cores
		extern portMUX_TYPE async_queue_mux;
		#define ASYNC_ENTER_CRITICAL()  portENTER_CRITICAL(&async_queue_mux)
		#define ASYNC_EXIT_CRITICAL()   portEXIT_CRITICAL(&async_queue_mux)
	#else
		#define ASYNC_ENTER_CRITICAL()  noInterrupts()
		#define ASYNC_EXIT_CRITICAL()   interrupts()
	#endif
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
		// ── Constructor / Destructor ──────────────────────────────
		explicit AsyncDevice(int* error);
		~AsyncDevice();

		// ── Public API ────────────────────────────────────────────
		int send(const str& payload);
		bool recv(str& out);
		void stop();
		void startRx();
		size_t getMTU();
		void onReceive();   // called from DIO1 interrupt

private:
		// ── Hardware interrupt processing (Now Private) ──────────
		void process();

		// ── Radio handle ─────────────────────────────────────────
		SX1262* _radio;

		#ifndef ARDUINO
		PiHal* hal;
		#endif

		SX1262* init_radio();

		// ── FreeRTOS Dual-Core Abstraction ───────────────────────
		#if defined(ARDUINO) && defined(ESP32)
		TaskHandle_t _processTaskHandle;
		SemaphoreHandle_t _spiMutex;
		static void _taskTrampoline(void* pvParameters);
		#endif

		// ── State flags ──────────────────────────────────────────
		ASYNC_VOLATILE bool _rxArmed;      
		ASYNC_VOLATILE bool _txActive;     
		ASYNC_VOLATILE bool _packetReady;  

		// ── FIFO ring-buffer ─────────────────────────────────────
		AsyncPacket _queue[ASYNC_QUEUE_DEPTH];
		ASYNC_VOLATILE uint8_t _head;   
		ASYNC_VOLATILE uint8_t _tail;   
		ASYNC_VOLATILE uint8_t _count;  

#if !defined(ARDUINO)
		std::mutex _mutex;  // protects the ring-buffer on Linux/RPi
#endif

		// ── Helpers ───────────────────────────────────────────────
		bool     queuePush(const uint8_t* data, uint8_t len);
		bool     queuePop(AsyncPacket& out);
		uint8_t  queueCount() const;

		static AsyncDevice* _instance;

#if defined(ARDUINO)
		static void IRAM_ATTR _isrTrampoline();
#else
		static void           _isrTrampoline();
#endif
};

#endif