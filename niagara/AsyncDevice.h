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

/**
 * `MTU` for LoRa device communication. This is the maximum amount of bytes
 * which can be contained in an `AsyncPacket` structure.
 */
static constexpr uint8_t  ASYNC_MTU          = 255;  
/**
 * @brief Amount of packet space in the FIFO stack.
 * 
 * This represents the amount of packets which can be received without
 * being processed.
 */
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

/**
 * @class AsyncPacket
 * @author Zanotti Enrico
 * @brief Packet type stored in the FIFO ring-buffer
 * 
 * This structure represents the raw data received through the radio module.
 */
struct AsyncPacket {
		uint8_t  data[ASYNC_MTU];
		uint8_t  len;
};

/**
 * @file AsyncDevice.h
 * @class AsyncDevice
 * @author Zanotti Enrico
 * @brief Manager for a LoRa device with an asynchronous receive.
 * 
 * This class manages the LoRa module through an underlying RadioLib instance
 * with predefined pins and parameters based on the platform, this class is thus not
 * to be considered portable across different platforms with different pinouts or chips
 * because it manages specifically either an ESP32 Heltec LoRa v3 dev board or a 
 * raspberry Pi with waveshare LoRaHAT installed on it. Any other configuration isn't
 * supported without small tweaks to the underlaying code. This choice has been made
 * for the sake of simplicity for the project this library is made for.
 * 
 * The `SX1262` chip, which is managed by this class, is always set to be in `RX_WAIT` state
 * until either data is sent or it is being stopped for power saving purposes.
 * When the `AsyncDevice` object is created, the LoRa chip is initialised and immediately put 
 * in an RX state. If data is then received, an `ISR` (Interrupt Service Routine) is called 
 * to save that data inside a `FIFO` stack, so that a call to the `recv(str)` method results
 * in the first received packet to be returned, and successive calls to the other packets in the stack
 * to be returned.
 * 
 * The ISR which is used to receive data from the `SX1262` module and save it into the FIFO ring-buffer,
 * calls a separate function to do so. This function, depending on the platform, is handled differently,
 * as it can't run in the ISR callback and must then run in a separate thread or core.
 * On the ESP32 platform, since that specific dev module has a dual core processor, FreeRTOS is used
 * to have a separate task pinned to the second core of that device, which handles asynchronous receiving
 * and saving, while on the Raspberry Pi platform running linux, a separate thread is created.
 * 
 * Given the initialization of a chip with specific pinouts and tasks pinned to specific cores, no more
 * than one instance of this class should be made per program.
 * Multiple object creation thus results in undefined behaviour.
 * 
 * When a transmission is initiated, the receiving state is stopped to set the chip in TX mode and
 * start transmitting the data. The chip is then put back in receiving state after it has completed.
 */
class AsyncDevice {
	public:
		/**
		 * @brief Class constructor
		 * @param error Pointer to an error code indicating any problem which
		 * 				might have occurred during initialisation of the LoRa chip.
		 * 
		 * This initialises the underlaying RadioLib instance and the LoRa chip,
		 * immediately putting it in RX mode.
		 */
		explicit AsyncDevice(int* error);
		~AsyncDevice();

		/**
		 * @brief Synchronous data send
		 * @param payload Data to send, in `str` format.
		 * @returns Any error code which might have occurred while sending the data
		 * 
		 * This method puts the chip out of RX mode to start a synchronous data send.
		 * 
		 * The chip is put back into RX mode after the transmission completed.
		 */
		int send(const str& payload);
		/**
		 * @brief Asynchronous data receive
		 * @param out Any data that might have been received
		 * @returns `true` if data was available for receive, `false` otherwise.
		 * 
		 * This method checks the internal FIFO stack and returns the first available element if it's not empty.
		 */
		bool recv(str& out);
		/**
		 * @brief Stops the chip
		 * 
		 * This puts the chip in `STANDBY` mode and stops the asynchronous receive, to save power.
		 */
		void stop();
		/**
		 * @brief Starts the chip back up from standby.
		 * 
		 * Used after a `stop()` call, it puts the chip back in receiving state.
		 */
		void startRx();
		/**
		 * @brief Getter for the chip's MTU (Maximum Transmission Unit)
		 * @returns The chip's MTU
		 * 
		 * This value identifies the maximum amount of bytes the 
		 * chip can send/receive at a time.
		 */
		size_t getMTU();
		/**
		 * @brief Callback for when data is received.
		 * 
		 * This is a reserved method which shouldn't be manually called.
		 */
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