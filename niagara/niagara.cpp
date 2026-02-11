#include "niagara.h"
#include "Timer.h"
#include "Hash.h"

#if defined(ARDUINO)
void Niagara::display_printf(const char* format, ...) {
  char buffer[1024];  // Puoi aumentare se ti serve più spazio

  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);  // formatta nel buffer
  va_end(args);

  display.print(buffer);  // stampa sul display
}

void Niagara::display_print(String text) {
  display.print(text);
}
#endif

#if defined(ARDUINO)
SX1262* Niagara::init_radio() {
  // SX1262 has the following connections on this board:
  // NSS pin:   8
  // DIO1 pin:  14
  // NRST pin:  12
  // BUSY pin:  13
  SX1262* radio = new SX1262(new Module(8, 14, 12, 13));

  if(Niagara::display_log) display.print("[SX1262] Initializing... ");
  int state = radio->begin(868.0);
  if(state != RADIOLIB_ERR_NONE) {
    if(Niagara::display_log) display_printf("\nInitialization Failed!\nError code: %d\n", state);
    return nullptr;
  }
  state = radio->setCRC(2);
  if(state != RADIOLIB_ERR_NONE) {
    if(Niagara::display_log) display_printf("\nCRC Initialization Failed!\nError code: %d\n", state);
    return nullptr;
  }
  if(Niagara::display_log) display.println("OK");
  return radio;
}
#else
SX1262* Niagara::init_radio() {
  // now we can create the radio module
  SX1262* radio = new SX1262(new Module(hal, 21, 16, 18, 20 /*The BUSY pin of the module MUST be specified, otherwise error -2 is thrown*/));
  
  if(Niagara::display_log) fprintf(stderr, "[SX1262] Initializing... ");
  /* The module is being initialized with all the default begin() settings
   * The only settings changed are the following:
   * - The frequency, according to the EU868 standard must be 868MHz, the default frequency is 434MHz
   * - The TCXO voltage, which is the crystal which is powering the clock, since this module is not using TCXO, 
   *   not specifying that will cause error -707 to be thrown, so we need to specify its voltage to be 0
   */
  int state = radio->begin(868.0 /*EU868 frequency*/, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 0 /*This is not the default value*/, false);
  if(state != RADIOLIB_ERR_NONE) {
    if(Niagara::display_log) fprintf(stderr, "Initialization Failed!\nError code: %d\n", state);
    	return nullptr;
  }
  if(Niagara::display_log) fprintf(stderr, "Initialization successful!\n");
  return radio;
}
#endif

Niagara::Niagara(bool log) {
  //Save the log flag the user specified
  Niagara::display_log = log;
  
  #if defined(ARDUINO)
  //Initialise the display and heltec library
  heltec_setup();
  display.init();
  display.setFont(ArialMT_Plain_10);
  #else
  // use SPI channel 1, because on Waveshare LoRaWAN Hat,
  // the SX1261 CS is connected to CE1
  Niagara::hal = new PiHal(1);
  #endif

  //Initialize the radio module
  SX1262* radio_return = init_radio();
  if(radio_return == nullptr) return;
  Niagara::lora = radio_return;
  Niagara::chip_mtu = lora->maxPacketLength();

  //Initialise the identifier as empty
  Niagara::identifier = "";
}

/* If the boolean for the log variable isn't defined, just set it to true */
Niagara::Niagara() : Niagara(true) {}

Niagara::~Niagara() {
  if(Niagara::lora) {
    delete Niagara::lora;
    Niagara::lora = nullptr;
  }
  
  #ifndef ARDUINO
  if(Niagara::hal) {
    delete Niagara::hal;
    Niagara::hal = nullptr;
  }
  #endif
}

bool Niagara::set_identifier(str _identifier) {
  if(!check_identifier(_identifier))
    return false;

  Niagara::identifier = _identifier;
  return true;
}

Niagara_Ret Niagara::receive(str* output, str* source) {
  //Check if the identifier hasn't yet been initialised
  if(identifier.length() == 0)
      return NIAGARA_NO_IDENTIFIER;

  //Saves the current receive state of the handshake  
  enum class RxState {
    WAIT_SYN,
    WAIT_FINAL_ACK
  };

  //Variable which keeps track of the handshake's state
  RxState state = RxState::WAIT_SYN;
  //Timer used to keep track of timeouts
  Timer session_timer;

  //The remote device's identifier to communicate with
  str device = "";
  //Buffer for the payload
  str payload;
  //Buffer for the control message
  Niagara_Control control;
  //Return status of send and receive methods
  Niagara_Ret status;

  //Start the timer to count for timeouts
  session_timer.start();

  //Keep reiterating until the amount of retransmissions reaches the maximum amount of retransmissions
  while(true) {
    //Check if the session time has exceeded the timeout in which case stop the communication
    if(session_timer.elapsed() > MAX_RECV_WAIT)
      return NIAGARA_TIMEOUT;

    //Receive raw data from the radio module and process it at the lower layer
    status = Niagara::receive_raw(source, &control, &payload);

    //If the receive went into timeout state then try receiving the data again
    if(status == NIAGARA_TIMEOUT)
      continue;

    //If the receive returned an error then propagate it to the whole method
    if(status != NIAGARA_OK)
      return NIAGARA_RECEIVE_ERROR;

    /*
     * Check if the remote device has been set and it's the source of this message,
     * in which case, if the control message is a SYNCHRONIZE request, reset the handshake
     * status to restart. This situation might happen when the CRC32 check failed on the other
     * side and the other side sent the message again with the control message set as a SYN.
    */
    if(device == *source && control == SYN)
      state = RxState::WAIT_SYN;

    //Check based on the current handshake's state
    switch(state) {
      //If the handshake is waiting for a SYN message and it got received
      case RxState::WAIT_SYN:
        //If the maximum retransmission amount was reached on the remote side then close
        if(control == RETRANSMISSION_TIMEOUT)
          return NIAGARA_RETRANSMISSION_ERROR;
        //Check if the control message is SYN, otherwise ignore this message
        if(control != SYN)
          break;

        //The remote device we're communicating with is the source of this message
        device = *source;

        //Reply with the acknowledgement 
        {
          //Compute the CRC to send back to the remote device
          str crc = crc32_to_str(crc32(payload));
          //Send the crc back as the payload of the acknowledgement
          if(Niagara::send_raw(device, ACK, crc) != NIAGARA_OK)
            return NIAGARA_SEND_ERROR; //In case of any send error then propagate it to the whole method
        }

        //Set the state to wait for the final acknowledgement
        state = RxState::WAIT_FINAL_ACK;
        //Reset the session timer
        session_timer.start();
        //Go back and wait for data to receive
        break;

      //If the handshake was pending for final acknowledgement
      case RxState::WAIT_FINAL_ACK:
        //Check if the source is the remote device we're communicating with
        if(*source != device)
          break;

        //If the control message matches with the requested acknowledgement
        if(control == ACK) {
          //Return the output and return the OK status
          *output = payload;
          return NIAGARA_OK;
        }

        //If the maximum retransmission amount was reached on the remote side then close
        if(control == RETRANSMISSION_TIMEOUT)
          return NIAGARA_RETRANSMISSION_ERROR;

        break;
      }
  }
}

Niagara_Ret Niagara::send(str destination, str message) {
  //Check if the identifier hasn't yet been initialised
  if(identifier.length() == 0)
    return NIAGARA_NO_IDENTIFIER;

  //Saves the current send state of the handshake  
  enum class TxState {
    SEND_SYN,
    WAIT_ACK
  };

  //Variable to keep track of the current handshake's state
  TxState state = TxState::SEND_SYN;
  //Timer to check for timeouts
  Timer timer;
  //Counter for the amount of retransmissions
  int retransmissions = 0;

  //Remote device's callsign
  str remote;
  //Buffer for the received crc
  str received_crc;
  //Buffer for the control message received
  Niagara_Control control;
  //Buffer for the status of send and receive functions
  Niagara_Ret status;

  //Compute the crc of the message being sent so it can be checked
  str expected_crc = crc32_to_str(crc32(message));

  //Start the timer counting for retransmissions
  timer.start();
  //Loop until the maximum amount of retransmissions is reached
  while(retransmissions < NIAGARA_RETRANSMISSIONS) {
    switch(state) { //Check the current state of the handshake
      case TxState::SEND_SYN: //If the SYN request should be sent:
        //Send the SYN request with the message as payload
        if(Niagara::send_raw(destination, SYN, message) != NIAGARA_OK)
          return NIAGARA_SEND_ERROR; //In case of any error propagate it to the whole method

        //Start the timer counting for retransmission timeouts
        timer.start();
        //Wait for the acknowledgement
        state = TxState::WAIT_ACK;
        break;

      //If the ACK should be received
      case TxState::WAIT_ACK:
        //Wait for it to be actually received
        status = Niagara::receive_raw(&remote, &control, &received_crc);

        //In case the acknowledgement timed out then try to retransmit the message
        if(status == NIAGARA_TIMEOUT && timer.elapsed() > MAX_RECV_WAIT) {
          //Increment the retransmission counter
          retransmissions++;
          //Go back to the SYN
          state = TxState::SEND_SYN;
          //Exit from the switch condition
          break;
        }

        //If the receive method returned an error then propagate it to the whole method
        if(status != NIAGARA_OK)
          return NIAGARA_RECEIVE_ERROR;

        //If the remote device isn't correct or the control message isn't an acknowledgement then ignore this message
        if(remote != destination || control != ACK)
          break;

        //If the CRC check doesn't work correctly then go back to the SYN and retransmit
        if(received_crc != expected_crc) {
          //Increase the retransmission counter
          retransmissions++;
          //Go back to sending the SYN request
          state = TxState::SEND_SYN;
          //Exit from the switch condition
          break;
        }

        //Send the final acknowledgement with no payload and exit
        status = Niagara::send_raw(destination, ACK, "");
        //If there has been a problem sending the message then propagate that problem back to the whole method
        if(status != NIAGARA_OK)
          return NIAGARA_SEND_ERROR;
        //If the final acknowledgement had no internal error then the message got sent correctly
        return NIAGARA_OK;
    }
  }

  //In case of no retransmissions left then send a retransmission timeout message and exit
  Niagara::send_raw(destination, RETRANSMISSION_TIMEOUT, "");
  return NIAGARA_TIMEOUT;
}

Niagara_Ret Niagara::receive_raw(str* source, Niagara_Control* control_output, str* message_output) {
  //If the identifier is empty and not yet initialized then return an error
  if(Niagara::identifier.length() == 0)
    return NIAGARA_NO_IDENTIFIER;

  char receive_output[Niagara::chip_mtu];
  int status = Niagara::lora->receive((uint8_t*)receive_output, Niagara::chip_mtu);
  if(status == RADIOLIB_ERR_RX_TIMEOUT) return NIAGARA_TIMEOUT;
  if(status != RADIOLIB_ERR_NONE) return RADIOLIB_ERROR;
  str receive_output_str(receive_output);
  str processed_output[3];
  //Check for errors on the process message method
  status = Niagara::process_message(processed_output, receive_output_str) != 0
  if(status == 6) return NIAGARA_NOT_DESTINATION;
  else if(status != 0) return NIAGARA_INVALID_DATA;

  int control_value = processed_output[1].toInt();

  *source = processed_output[0];
  *control_output = static_cast<Niagara_Control>(control_value);
  *message_output = processed_output[2];

  return NIAGARA_OK;
}

int Niagara::process_message(str* output, str message) {
  //If the identifier is empty and not yet initialized then return an error
  if(identifier.length() == 0)
    return 1;
  if(message.length() == 0)
    return 2;

  //In case the message destination isn't this board then exit
  int separatorIndex = message.indexOf('|');
  if(separatorIndex <= 0) return 3;
  str callsign = message.substring(0, separatorIndex);
  //Separate the destination id in source and destination id
  str sourceID, destinationID;
  int identifierSeparator = callsign.indexOf('.');
  if(identifierSeparator <= 0 || identifierSeparator >= callsign.length() - 1) return 4;
  if(callsign.length() <= identifierSeparator) return 5;
  sourceID = callsign.substring(0, identifierSeparator);
  destinationID = callsign.substring(identifierSeparator + 1);
  if(!valid_destination(destinationID)) return 6;
  if(message.length() <= separatorIndex + 1) return 7; 
  int secondSeparatorIndex = message.substring(separatorIndex + 1).indexOf('|') + separatorIndex + 1;
  if(secondSeparatorIndex <= 0) return 8;
  
  //Separate the str into the control sequence and the message itself
  output[0] = sourceID;
  output[1] = message.substring(separatorIndex + 1, secondSeparatorIndex);
  if(message.length() > secondSeparatorIndex + 1)
    output[2] = message.substring(secondSeparatorIndex + 1);
  else output[2] = "";

  return 0;
}

Niagara_Ret Niagara::send_raw(str destination, Niagara_Control control, str message) {
  //If the identifier is empty and not yet initialized then return an error
  if(Niagara::identifier.length() == 0)
    return NIAGARA_NO_IDENTIFIER;

  str formattedMessage = Niagara::format_message(destination, control, message);
  if(formattedMessage.length() == 0) return NIAGARA_INVALID_DATA;
  //Check if the formatted message to send is bigger than the maximum transmission unit to send over the radio
  if(formattedMessage.length() >= Niagara::chip_mtu) return NIAGARA_TOO_LARGE;
  int status = Niagara::lora->transmit(formattedMessage.c_str());
  if(status == RADIOLIB_ERR_TX_TIMEOUT) return NIAGARA_TIMEOUT;
  if(status != RADIOLIB_ERR_NONE)
    return RADIOLIB_ERROR;
  return NIAGARA_OK;
}

str Niagara::format_message(str destination, Niagara_Control control, str message) {
    if(identifier.length() == 0)
        return "";

    if(destination.length() == 0)
        return "";

    if(destination.indexOf('|') >= 0 || destination.indexOf('.') >= 0)
        return "";

    if(message.indexOf('|') >= 0)
        return "";

    if(control < 0 || control >= END)
        return "";

    return identifier + "." + destination + "|" +
           str(static_cast<int>(control)) + "|" +
           message;
}

bool Niagara::valid_destination(str destination) {
  //Check for broadcast destination
  if(destination == BROADCAST)
    return true;

  //Check if the two strings match in size, if they don't then skip
  if(Niagara::identifier.length() != destination.length()) return false;

  #if defined(ARDUINO)
  for (size_t i = 0; i < Niagara::identifier.length(); ++i) {
      if (tolower(Niagara::identifier[i]) != tolower(destination[i])) return false;
  }
  #else
  for (size_t i = 0; i < Niagara::identifier.length(); ++i) {
      if (std::tolower(static_cast<unsigned char>(Niagara::identifier[i])) !=
          std::tolower(static_cast<unsigned char>(destination[i]))) {
          return false;
      }
  }
  #endif

  return true;
}

bool Niagara::check_identifier(str identifier) {
  //The identifier should be between 4 and 12 characters long
  size_t len = identifier.length();
  if (len < 4 || len > 12) return false;
  if(identifier == BROADCAST) return false;

  //Check if the identifier contains only alphanumeric values
  #if defined(ARDUINO)
  for (int i = 0; i < len; ++i) {
      char c = identifier[i];
      if (!isAlphaNumeric(c)) return false;
  }
  return true;
  #else
  const char* identifier_array = identifier.c_str();
  for (int i = 0; i < identifier.length(); i++) {
      if (!std::isalnum(static_cast<unsigned char>(identifier_array[i]))) return false;
  }
  return true;
  #endif
}
