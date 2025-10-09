#ifndef NIAGARA_H
#define NIAGARA_H

// include the library
#include <RadioLib/RadioLib.h>
#include <optional>
//Used for printf
#include <stdarg.h>
#include <string>
// include the hardware abstraction layer
#include "hal/RPi/PiHal.h"

#define BROADCAST "BROAD"
#define NIAGARA_RETRANSMISSIONS 10

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

class NiagaraPi {
  public:    
    NiagaraPi(std::string _identifier, bool log);
    NiagaraPi(std::string _identifier);
    
    /**
     * Waits for an incoming connection from a device and
     * accepts it.
     */
    Niagara_Ret listen();
    /**
     * Sends a connection request to another specified
     * device.
     */
    Niagara_Ret connect(std::string identifier);
    /**
     * Closes an established connection with the device
     */
    Niagara_Ret end();
    
  private:
    /*Receives a message from the LoRa device*/
    Niagara_Ret receive(std::string* source, Niagara_Control* control_output, std::string* message_output);

    /*Sends a message to a specific destination*/
    Niagara_Ret send(std::string destination, Niagara_Control control, std::string message);

    SX1262* lora;
    std::string identifier;
    bool display_log;
    
    // instance of the HAL class
    PiHal* hal;

    /*
    * This method handles all needed initializations to create the
    * object used to manage the radio module.
    */
    std::optional<SX1262> init_radio();

    /*Used to process messages received using the protocol*/
    std::string* process_message(std::string message);

    /*Used to format messages according to the protocol
     * A blank std::string is returned in case invalid parameters are passed in destination message
     * or control message.
    */
    std::string format_message(std::string destination, std::string control, std::string message);
};

#endif