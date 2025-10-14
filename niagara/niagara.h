#ifndef NIAGARA_H
#define NIAGARA_H

#if defined(ARDUINO)
// include the library
#include <RadioLib.h>
//Used for display.println
#include <heltec_unofficial.h>
#else
// include the library
#include <RadioLib/RadioLib.h>
#include <string>
// include the hardware abstraction layer
#include "hal/RPi/PiHal.h"
#endif

#include <optional>
//Used for printf
#include <stdarg.h>

#define BROADCAST "BROAD"
#define NIAGARA_RETRANSMISSIONS 10

#if defined(ARDUINO)
typedef str String;
#else
typedef str std::string;
#endif

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
     * Returned by the transmit method when the
     * destination device string or the control message are
     * invalid, or by the transmit method when the
     * destination string is valid but the control message
     * received isn't.
    */
    NIAGARA_INVALID_DATA,    
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
     * When an error occurred while sending
     */
    NIAGARA_SEND_ERROR
}

/**
 * This contains the possible control messages.
 * 
 * The enumerator's elements must all be ascending constantly from zero.
 */
enum Niagara_Control {
    HANDSHAKE_SYN,
    HANDSHAKE_ACK,
    CONTROL_PING,
    CONTROL_REQUEST_DATA,
    CONTROL_RESPONSE,
    TIME_SYNC,
    END //This must be the last element
}

class Niagara {
  public:    
    Niagara(str _identifier, bool log);
    Niagara(str _identifier);

    #if defined(ARDUINO)
    /*Custom display print methods */
    void display_printf(const char* format, ...);
    void display_print(String text);
    #endif
    
    /**
     * Waits for an incoming connection from a device and
     * accepts it.
     */
    Niagara_Ret listen();
    /**
     * Sends a connection request to another specified
     * device.
     */
    Niagara_Ret connect(str identifier);
    /**
     * Closes an established connection with the device
     */
    Niagara_Ret end();
    
  private:
    /*Receives a message from the LoRa device*/
    Niagara_Ret receive(str* source, Niagara_Control* control_output, str* message_output);

    /*Sends a message to a specific destination*/
    Niagara_Ret send(str destination, Niagara_Control control, str message);

    SX1262* lora;
    str identifier;
    bool display_log;
    
    // instance of the HAL class
    PiHal* hal;

    /*
    * This method handles all needed initializations to create the
    * object used to manage the radio module.
    */
    std::optional<SX1262> init_radio();

    /*Used to process messages received using the protocol*/
    str* process_message(str message);

    /*Used to format messages according to the protocol
     * A blank str is returned in case invalid parameters are passed in destination message
     * or control message.
    */
    str format_message(str destination, str control, str message);
};

#endif