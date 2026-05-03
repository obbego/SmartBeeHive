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
	// Raspberry Pi (Linux)
	#include <RadioLib/RadioLib.h>
	#include "hal/RPi/PiHal.h"
	#include <string>
	#include <cstring>
	#include <atomic>
	#include <mutex>
	#include <thread>
#endif

#include <stdint.h>
#include "str.h"

// ── Buffer configuration ────────────────────────────────────
static constexpr uint8_t  ASYNC_MTU          = 255;  
static constexpr uint8_t  ASYNC_QUEUE_DEPTH  = 8;    

// ── Platform-specific ISR / atomic helpers ──────────────────
#if defined(ARDUINO)
	#if defined(ESP32)
		extern portMUX_TYPE async_queue_mux;
		#define ASYNC_ENTER_CRITICAL()  portENTER_CRITICAL(&async_queue_mux)
		#define ASYNC_EXIT_CRITICAL()   portEXIT_CRITICAL(&async_queue_mux)
	#else
		#define ASYNC_ENTER_CRITICAL()  noInterrupts()
		#define ASYNC_EXIT_CRITICAL()   interrupts()
	#endif
	#define ASYNC_VOLATILE          volatile
#else
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
		explicit AsyncDevice(int* error);
		~AsyncDevice();

		int send(const str& payload);
		bool recv(str& out);
		void stop();
		void startRx();
		size_t getMTU();
		void onReceive();

private:
		void process();

		SX1262* _radio;

		#ifndef ARDUINO
		PiHal* hal;
		#endif

		SX1262* init_radio();

		// ── Concurrency Abstractions ─────────────────────────────
		#if defined(ARDUINO) && defined(ESP32)
		TaskHandle_t _processTaskHandle;
		SemaphoreHandle_t _spiMutex;
		static void _taskTrampoline(void* pvParameters);
		#elif !defined(ARDUINO)
		std::thread* _processThread;
		std::atomic<bool> _threadRunning;
		std::mutex _linuxSpiMutex;
		std::mutex _queueMutex;
		static void _threadTrampoline(AsyncDevice* instance);
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