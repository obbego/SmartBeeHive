/**
 * @file niagara.h
 * @brief Libreria di gestione comunicazione via LoRa per Raspberry Pi o Arduino con sottosistema RadioLib
 */

#ifndef NIAGARA_H
#define NIAGARA_H

#if defined(ARDUINO)
//Used for printf
#include <stdarg.h>
#else
#include <string>
#include <cctype>
#endif

#include "AsyncDevice.h"
#include <stdarg.h>
#include "str.h"

/**
 * Callsign for broadcast communications.
 * This callsign can only be set as a destination
 */
#define BROADCAST "BROAD"
/** Maximum amount of retransmissions to be sent before the send method throws an error */
#define NIAGARA_RETRANSMISSIONS 10
/** Amount of time that the handshake waits for the other device to reply before trying a retransmission */
#define MAX_RECV_WAIT 5000
/** Amount of ms to check for received data in a loop during receive operations in the handshake.
 * Decreasing this value increases responsiveness and possible lost packages but increases load.
 */
#define RECEIVE_CHECK_MS 100

/**
 * @typedef NiagaraLogHandler
 * @brief Callback for the log handler
 * 
 * This callback must be a function which accepts as input a `const char*` type and
 * should contain logic to handle log printing to a terminal or an external display.
 */
typedef void (*NiagaraLogHandler)(const char*);

/**
 * @enum Niagara_Ret
 * @brief Contains the return value of functions of the Niagara library
 * 
 * This enumerator is returned by any method of the
 * library indicating any error occurred during any operation.
 */
enum Niagara_Ret {
		/**
		 * No error encountered
		 */
		NIAGARA_OK,    
		/**
		 * Returned by a method when it's called when no identifier
		 * has yet been set for this device using set_identifier(str)
		 */
		NIAGARA_NO_IDENTIFIER,
		/**
		 * Returned by the transmit method when the
		 * destination device string or the control message are
		 * invalid, or when the destination string is valid but 
		 * the control message received isn't.
		*/
		NIAGARA_INVALID_DATA,    
		/**
		 * Returned by receive methods in case no data is available to receive.
		 */
		NIAGARA_NO_DATA,
		/**
		 * Returned by the underlaying send method when the binary data
		 * which is to be sent over the radio exceedes the maximum size
		 * which can be sent at a time.
		*/
		NIAGARA_TOO_LARGE,
		/**
		 * Returned by the raw receive method when the
		 * message received's destination isn't this device.
		 * 
		 * Otherwise it's returned by the send method when the destination
		 * passed isn't a valid identifier.
		*/
		NIAGARA_NOT_DESTINATION,
		/**
		 * Returned when the process timed out
		 */
		NIAGARA_TIMEOUT,
		/**
		 * Error returned when the internal library
		 * gave an error during the process.
		 */
		RADIOLIB_ERROR,
		/**
		 * When an error occurred while receiving
		 */
		NIAGARA_RECEIVE_ERROR,
		/**
		 * Returned by the receive method when the maximum amount 
		 * of retransmissions was reached by the sending device.
		*/
		NIAGARA_RETRANSMISSION_ERROR,
		/**
		 * When an error occurred while sending
		 */
		NIAGARA_SEND_ERROR
};

/**
 * @enum Niagara_Control
 * @brief Contains the possible control messages.
 * 
 * The elements of this enumerator represent the possible values
 * which can be present in the second parameter after the pipe separator
 * in the protocol's raw (lowest) layer.
 */
enum Niagara_Control {
		SYN,
		ACK,
		RETRANSMISSION_TIMEOUT,
		NONE,
		END //This must be the last element
};

/**
 * @enum Niagara_LogLevel
 * @brief Defines the log level which the library should use to output logs to the callback
 * 
 */
enum Niagara_LogLevel {
		/** Used when the log should be concise because it's being output to a display */
		LOG_DISPLAY,
		/** Used when the log can be extended because it's being output to a terminal */
		LOG_TERMINAL, 
		/** Used when no log should be used. */
		NO_LOG
};

/**
 * @class Niagara
 * @brief Main class which implements the protocol
 * 
 * This class implements sending and receiving packets using handshakes
 * and retransmissions for redundancies, and fragmentation for a robust
 * communication using RadioLib as a backend.
 */
class Niagara {
	public:    
		/**
		 * @brief Initialization of the logged communication
		 * 
		 * Initialises this device with the log handler, which
		 * is a callback function which contains the method used
		 * to print the protocol's logs.
		 * 
		 * @param _log_handler Callback function for external logging
		 * @param _log_level The log level which should be used to communicate to the callback function
		 */
		Niagara(NiagaraLogHandler _log_handler, Niagara_LogLevel _log_level);
		/**
		 * @brief Default empty constructor without logs
		 * 
		 * This constructor doesn't implement any logging in the object
		 */
		Niagara();
		~Niagara();

		/**
		 * @brief Receives the first available message.
		 * 
		 * Waits for an incoming message and accepts each fragment of it,
		 * if more than one. Then reconstructs the final message, outputting
		 * it and its source.
		 * 
		 * The `source` string contains the callsign of the remote device which
		 * has established a connection.
		 * 
		 * @param output Pointer to the string which will store the content of the message received
		 * @param source Pointer to the string which will store the source of the message received
		 *               This can be `nullptr`, it will just avoid storage of the message source.
		 * @returns A `Niagara_Ret` object representing any error which might've happened 
		 *          while receiving or defragmenting the packet.
		 */
		Niagara_Ret receive(str* output, str* source = nullptr);
		/**
		 * @brief Sends a message to the provided destination.
		 * 
		 * 
		 * Sends a message to a remote device, fragmenting it if
		 * needed.
		 * 
		 * @param destination Callsign to message's destination, can also be `BROADCAST`.
		 * @param message The message to send, which, if exceedes the chip's MTU, will be fragmented into multiple packets.
		 * @returns A `Niagara_Ret` object representing any error which might've happened 
		 *          while transmitting or fragmenting the packet.
		 */
		Niagara_Ret send(str destination, str message);

		/**
		 * @brief Sets the identifier for this device and must 
		 *        be called before calling any send or receive method.
		 * 
		 * The identifier, or *callsign* of the device should follow these formatting rules:
		 *     - Must be between `4` and `12` characters long
		 *     - Mustn't be `BROADCAST`
		 *     - Must be alphanumeric, no special characters allowed
		 * 
		 * Not setting an identifier before calling `receive(str*, str*)` or `send(str, str)`
		 * will result in `NIAGARA_NO_IDENTIFIER` to be returned by those methods.
		 * 
		 * @param identifier The identifier, or callsign, to set
		 * @returns `true` if the identifier was set correctly, otherwise `false`.
		 */
		bool set_identifier(str identifier);
		
	private:
		/* Contacts the radio chip and sets in it RX receive mode, so that from this method's call
		 * it can receive data asynchronously, which can then be received later.
		 */
		int start_receive_raw();
		/*
		 * If data is available for reception, then it returns it.
		 * This method must be called after the chip has started an asynchronous reception
		 * through start_receive_raw().
		*/
		Niagara_Ret receive_raw(str* source, Niagara_Control* control_output, str* message_output);
		/*Sends a raw message to a specific destination */
		Niagara_Ret send_raw(str destination, Niagara_Control control, str message);
		
		/**
		 * Waits for an incoming packet from a device and
		 * accepts it. This method doesn't handle
		 * packet fragments so it's limited to the device's MTU.
		 * 
		 * A filter can be specified in case it's required to receive a packet
		 * from a specific source, which callsign can be set in there, otherwise, 
		 * if not set, this method will just receive the first packet available.
		 */
		Niagara_Ret receive_fragment(str* output, str* source, str filter = "");
		/**
		 * Sends a packet handshake to another specified
		 * device. This method doesn't handle
		 * packet fragments so it's limited to the device's MTU.
		 */
		Niagara_Ret send_fragment(str destination, str message);

		/*Log print methods which use the handler if available. */
		void log_printf(const char* format, ...);
		void log_printf(Niagara_LogLevel level, const char* format, ...);
		void log_print(str text);
		void log_print(str extendedText, str conciseText);
		void log_print(Niagara_LogLevel level, str text);

		// Helper function which doesn't uses ellipsis for variadicity
		void vlog_printf(const char* format, va_list args);

		/* RadioLib pointer to the LoRa chip */
		AsyncDevice* lora = nullptr;
		/* Niagara identifier for this device */
		str identifier;
		/* If this value is set to something other than nullptr,
		then the log is enabled. */
		NiagaraLogHandler log_handler = nullptr;
		/* This value determines how much the log should be concise.
		*/
		Niagara_LogLevel log_level = NO_LOG;
		/* Chip's hardware MTU */
		uint16_t chip_mtu;

		/*Used to process messages received using the protocol*/
		int process_message(str* output, str message);

		/*Used to format messages according to the protocol
		 * A blank str is returned in case invalid parameters are passed in destination message
		 * or control message.
		*/
		str format_message(str destination, Niagara_Control control, str message);

		/**
		 * Given the destination parameter of a message received,
		 * this method checks if the destination parameter matches
		 * this device.
		 */
		bool valid_destination(str destination);

		/**
		 * Method used to check the validity of the identifier
		 * given to the constructor.
		 * 
		 * The second parameter defines whether the method is being used
		 * to check an identifier being sent. If this value is false, 
		 * the method will return invalid identifier if the identifier passed is
		 * broadcast.
		 */
		bool check_identifier(str identifier, bool sending);

		/**
		 * Function which cleans the received crc and checks
		 * if it's equal to the expected one, returning true if so.
		 */
		bool check_crc(str received_crc, str expected_crc);
};

#endif
