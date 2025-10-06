// include the library
#include <RadioLib/RadioLib.h>
#include <optional>
//Used for printf
#include <stdarg.h>
#include <string>
// include the hardware abstraction layer
#include "hal/RPi/PiHal.h"

/*Returned by the receive method when the
 * message received's destination isn't this device
*/
#define NIAGARA_NOT_DESTINATION 10
/*
 * Returned by the transmit method when the
 * destination device string or the control message are
 * invalid, or by the transmit method when the
 * destination string is valid but the control message
 * received isn't.
*/
#define NIAGARA_INVALID_DATA 11

#define CONTROL_REQUEST_DATA "A"
#define CONTROL_RESPONSE "B"
#define CONTROL_PING "C"

class NiagaraPi {
  public:    
    NiagaraPi(std::string _identifier, bool log);
    NiagaraPi(std::string _identifier);

    /*Receives a message from the LoRa device*/
    int receive(std::string* source, std::string* control_output, std::string* message_output);

    /*Sends a message to a specific destination*/
    int send(std::string destination, std::string control, std::string message);

  private:
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
