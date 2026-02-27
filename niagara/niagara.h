#ifndef NIAGARA_H
#define NIAGARA_H

#if defined(ARDUINO)
// include the library
#include <RadioLib.h>
//Used for printf
#include <stdarg.h>
#else
// include the library
#include <RadioLib/RadioLib.h>
#include <string>
#include <cctype>
// include the hardware abstraction layer
#include "hal/RPi/PiHal.h"
#endif

#include "str.h"

// To be set in the destination for broadcast communication
#define BROADCAST "BROAD"
// Maximum amount of retransmissions to be sent before the send method throws an error
#define NIAGARA_RETRANSMISSIONS 10
// Amount of time that the handshake waits for the other device to reply before trying a retransmission
#define MAX_RECV_WAIT 30000

//Define the type for the callback method of the log handler
typedef void (*NiagaraLogHandler)(const char*);

/**
 * This enumerator is returned by any method of the
 * library indicating any error occurred during any operation.
 */
enum Niagara_Ret {
    /*
     * No error encountered
     */
    NIAGARA_OK,    
    /*
     * Returned by a method when it's called when no identifier
     * has yet been set for this device using set_identifier(str)
     */
    NIAGARA_NO_IDENTIFIER,
    /*
     * Returned by the transmit method when the
     * destination device string or the control message are
     * invalid, or by the transmit method when the
     * destination string is valid but the control message
     * received isn't.
    */
    NIAGARA_INVALID_DATA,    
    /*
     * Returned by the underlaying send method when the binary data
     * which is to be sent over the radio exceedes the maximum size
     * which can be sent at a time.
    */
    NIAGARA_TOO_LARGE,  
    /*Returned by the receive method when the
     * message received's destination isn't this device
    */
    NIAGARA_NOT_DESTINATION,
    /*
     * Returned when the process timed out
     */
    NIAGARA_TIMEOUT,
    /*
     * Error returned when the internal library
     * gave an error during the process.
     */
    RADIOLIB_ERROR,
    /*
     * When an error occurred while receiving
     */
    NIAGARA_RECEIVE_ERROR,
    /*
     * Returned by the receive method when the maximum amount 
     * of retransmissions was reached by the sending device.
    */
    NIAGARA_RETRANSMISSION_ERROR,
    /*
     * When an error occurred while sending
     */
    NIAGARA_SEND_ERROR,
    /*
     * Returned when a receive function which is defragmenting a message
     * encounters a parsing problem which forces the fragment stream
     * to be halted.
    */
    NIAGARA_INVALID_FRAGMENT
};

/**
 * This contains the possible control messages.
 * 
 * The enumerator's elements must all be ascending constantly from zero.
 */
enum Niagara_Control {
    SYN,
    ACK,
    RETRANSMISSION_TIMEOUT,
    END //This must be the last element
};

enum Niagara_LogLevel {
    DISPLAY, //Used when the log should be concise because it's being output to a display
    TERMINAL, //Used when the log can be extended because it's being output to a terminal
    NONE //Used when no log should be used.
};

class Niagara {
  public:    
    /**
     * Initialises this device with the log handler, which
     * is a callback function which contains the method used
     * to print the protocol's logs.
     */
    Niagara(NiagaraLogHandler _log_handler, Niagara_LogLevel _log_level);
    Niagara();
    ~Niagara();

    /**
     * Waits for an incoming message and accepts each fragment of it,
     * if more than one. Then reconstructs the final message, outputting
     * it and its source.
     */
    Niagara_Ret receive(str* output, str* source);
    /**
     * Sends a message to a remote device, fragmenting it if
     * needed.
     */
    Niagara_Ret send(str destination, str message);

    /**
     * Sets the identifier for this device, this method must 
     * be called before calling any send or receive method.
     */
    bool set_identifier(str identifier);
    
  private:
    /*Receives a raw message from the LoRa device */
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
    Niagara_Ret receive_fragment(str* output, str* source, str filter);
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
    SX1262* lora = nullptr;
    /* Niagara identifier for this device */
    str identifier;
    /* If this value is set to something other than nullptr,
    then the log is enabled. */
    NiagaraLogHandler log_handler = nullptr;
    /* This value determines how much the log should be concise.
    */
    Niagara_LogLevel log_level = NONE;
    /* Chip's hardware MTU */
    uint16_t chip_mtu;
    
    #ifndef ARDUINO
    // instance of the HAL class
    PiHal* hal;
    #endif

    /*
    * This method handles all needed initializations to create the
    * object used to manage the radio module.
    */
    SX1262* init_radio();

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
     * given to the constructor
     */
    bool check_identifier(str identifier);

    /**
     * Function which cleans the received crc and checks
     * if it's equal to the expected one, returning true if so.
     */
    bool check_crc(str received_crc, str expected_crc);
};

#endif
