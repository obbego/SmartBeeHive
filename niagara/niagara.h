#ifndef NIAGARA_H
#define NIAGARA_H

#if defined(ARDUINO)
// include the library
#include <RadioLib.h>
//Used for display.println
#include <heltec_unofficial.h>
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
// Maximum amount of bytes which can be sent at a time on the radio device
#define LORA_BUFFER_MTU 512

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
    NIAGARA_SEND_ERROR
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

class Niagara {
  public:    
    /**
     * Initialises this device with the passed identifier,
     * which must be an alphanumeric string with 6-12 characters.
     */
    Niagara(bool log);
    Niagara();
    ~Niagara();

    #if defined(ARDUINO)
    /*Custom display print methods */
    void display_printf(const char* format, ...);
    void display_print(String text);
    #endif
    
    /**
     * Waits for an incoming connection from a device and
     * accepts it.
     */
    Niagara_Ret receive(str* output, str* source);
    /**
     * Sends a connection request to another specified
     * device.
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

    /* RadioLib pointer to the LoRa chip */
    SX1262* lora;
    /* Niagara identifier for this device */
    str identifier;
    /* Whether to log radio initialization or not */
    bool display_log;
    
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
    bool process_message(str* output, str message);

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
};

#endif
