#include "niagara.h"
#include "Timer.h"
#include "Hash.h"

#if defined(ARDUINO)
bool isValidInteger(String input) {
  boolean isNum=false;
  for(byte i=0;i<str.length();i++) {
    isNum = isDigit(str.charAt(i)) || str.charAt(i) == '+' || str.charAt(i) == '.' || str.charAt(i) == '-';
    if(!isNum) return false;
  }
  return isNum;
}
#endif

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
std::optional<SX1262> NiagaraESP::init_radio() {
  // SX1262 has the following connections on this board:
  // NSS pin:   8
  // DIO1 pin:  14
  // NRST pin:  12
  // BUSY pin:  13
  SX1262 radio = new Module(8, 14, 12, 13);

  if(Niagara::display_log) display.print("[SX1262] Initializing... ");
  int state = radio.begin(868.0);
  if(state != RADIOLIB_ERR_NONE) {
    if(Niagara::display_log) display_printf("\nInitialization Failed!\nError code: %d\n", state);
    return std::nullopt;
  }
  state = radio.setCRC(2);
  if(state != RADIOLIB_ERR_NONE) {
    if(Niagara::display_log) display_printf("\nCRC Initialization Failed!\nError code: %d\n", state);
    return std::nullopt;
  }
  if(Niagara::display_log) display.println("OK");
  return radio;
}
#else
std::optional<SX1262> Niagara::init_radio() {
  // now we can create the radio module
  SX1262 radio = new Module(hal, 21, 16, 18, 20 /*The BUSY pin of the module MUST be specified, otherwise error -2 is thrown*/);
  
  if(Niagara::display_log) fprintf(stderr, "[SX1262] Initializing... ");
  /* The module is being initialized with all the default begin() settings
   * The only settings changed are the following:
   * - The frequency, according to the EU868 standard must be 868MHz, the default frequency is 434MHz
   * - The TCXO voltage, which is the crystal which is powering the clock, since this module is not using TCXO, 
   *   not specifying that will cause error -707 to be thrown, so we need to specify its voltage to be 0
   */
  int state = radio.begin(868.0 /*EU868 frequency*/, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 0 /*This is not the default value*/, false);
  if(state != RADIOLIB_ERR_NONE) {
    if(Niagara::display_log) fprintf(stderr, "Initialization Failed!\nError code: %d\n", state);
    return std::nullopt;
  }
  if(Niagara::display_log) fprintf(stderr, "Initialization successful!\n");
  return radio;
}
#endif

Niagara::Niagara(bool log = true) {
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
  std::optional<SX1262> radio_return = init_radio();
  if(!radio_return.has_value()) return;
  Niagara::lora = &radio_return.value();

  //Initialise the identifier as empty
  Niagara::identifier = "";
}

/* If the boolean for the log variable isn't defined, just set it to true */
Niagara::Niagara() : Niagara(true) {}

bool Niagara::set_identifier(str _identifier) {
  if(!check_identifier(_identifier))
    return false;

  Niagara::identifier = _identifier;
  return true;
}

Niagara_Ret Niagara::receive(str* output, str* source) {
  //If the identifier is empty and not yet initialized then return an error
  if(Niagara::identifier.length() == 0)
    return NIAGARA_NO_IDENTIFIER;

  //Contains the ID of the first device whose first SYN was received
  str device;
  //Saves the received control signals
  Niagara_Control control;
  //Saves the output of the receivings
  str output_internal;
  //Contains the status of the operations
  Niagara_Ret status;
  //Amount of retransmissions done during the handshake
  int retransmission_counter = 0;
  //Flag to receive the message a second time since the hash didn't match
  bool reperform_receive;

  //Timer to check for retransmissions
  Timer retransmission_timer;

  //Wait until a receive has completed
  do {
    //Reset the flag to reperform the receive
    reperform_receive = false;

    /* If the receiving is being reperformed then don't handle
     * it again, work on the output of the last receive call.
    */
    if(!reperform_receive) {
      //Try to receive data
      status = Niagara::receive_raw(source, &control, &output_internal);
      //In case an error occurred then return an error
      if(status != NIAGARA_OK)
        return NIAGARA_RECEIVE_ERROR;
      
      //If not a syn req, skip
      if(control != SYN)
        continue;
    }

    //Compute hashed data for error check
    uint32_t hashed_data_int = crc32(output_internal);
    str hashed_data = (char*)&hashed_data_int;
    
    //While cycle for retransmissions
    retransmission_timer.start();
    //Set to false when a retransmission is not necessary
    bool retransmit = true;
    while(retransmission_counter < NIAGARA_RETRANSMISSIONS) {
      if(retransmit) {
        retransmit = true;
        //Send the first acknowledgement
        if(Niagara::send_raw(device, ACK, hashed_data) != NIAGARA_OK)
          return NIAGARA_SEND_ERROR;
      }
        
      //Try to receive new data, check the source
      str source;
      status = Niagara::receive_raw(&source, &control, &output_internal);
      if(status == NIAGARA_TIMEOUT) {
        retransmission_counter++;
        continue;
      }
      if(status != NIAGARA_OK ) {
        if(retransmission_timer.elapsed() > NIAGARA_TIMEOUT) {
          retransmission_timer.start();
          retransmission_counter++;
          continue;
        }

        retransmit = false;
        continue;
      }
      if(control == RETRANSMISSION_TIMEOUT)
        return NIAGARA_RECEIVE_ERROR;
      
      //If the source isn't the device which is handshaking or the control message isn't a ping then ignore
      if(source != device)
        continue;

      /* If another syn request was sent, then
       * reperform the receiving to handle the message that
       * has been resent.
       */
      if(control == SYN) {
        //Perform the receive again
        reperform_receive = true;
        break;
      }

      //If the control message is different then ignore the message
      if(control != ACK)
        continue;
    }

    //If the max retransmission amount was reached then ignore
    if(retransmission_counter == NIAGARA_RETRANSMISSIONS)
      return NIAGARA_TIMEOUT;
    else if(!reperform_receive) break; //If everything went well, then restart the cycle
  } while(reperform_receive);
  
  //Save the internal output in the actual output
  *output = output_internal;

  
  return NIAGARA_OK;
}

Niagara_Ret Niagara::send(str destination, str message) {
  //If the identifier is empty and not yet initialized then return an error
  if(Niagara::identifier.length() == 0)
    return NIAGARA_NO_IDENTIFIER;

  //Source of the receivings
  str source;
  //Received control message
  Niagara_Control control_msg;
  //Buffer to save the payload
  str crc_buffer;
  //Timer to keep track of retransmissions
  Timer retransmission_timer;
  //Retransmission counter
  int retransmission_counter = 0;
  //Status code returned by methods
  Niagara_Ret status;

  //Set to false when a retransmission is not necessary
  bool retransmit = true;
  //Start retransmission timer
  retransmission_timer.start();
  while(retransmission_counter < NIAGARA_RETRANSMISSIONS) {
    if(retransmit) {
      retransmit = true;
      //Send SYN to establish connection
      if(Niagara::send_raw(destination, SYN, message) != NIAGARA_OK)
        return NIAGARA_SEND_ERROR;
    }

    //Wait for acknowledgement
    status = Niagara::receive_raw(&source, &control_msg, &crc_buffer);
    if(status == NIAGARA_TIMEOUT) {
      retransmission_counter++;
      continue;
    }
    if(status != NIAGARA_OK)
      return NIAGARA_RECEIVE_ERROR;

    //Check that source and received parameters match with an acknowledgement
    if(source != destination || control_msg != ACK) {
      if(retransmission_timer.elapsed() > NIAGARA_TIMEOUT)
      {
        retransmission_timer.start();
        retransmission_counter++;
        continue;
      }

      retransmit = false;
      continue;
    }

    //Compare the crc with the one received from the receiving device
    uint32_t computed_crc_num = crc32(message);
    str computed_crc = (char*)&computed_crc_num;
    if(computed_crc != crc_buffer) {
      //Retransmit the message
      retransmission_timer.start();
      retransmission_counter++;
      continue;
    } 
  }

  //Check if the amount of retransmissions got exceeded
  if(retransmission_counter == NIAGARA_RETRANSMISSIONS) {
    Niagara::send_raw(destination, RETRANSMISSION_TIMEOUT, ""); //Send a message indicating maximum amount of retransmissions reached
    return NIAGARA_TIMEOUT;
  }

  //Send the last acknowledgement to finalize the handshake
  if(Niagara::send_raw(destination, ACK, "") != NIAGARA_OK)
    return NIAGARA_SEND_ERROR;

  return NIAGARA_OK;
}

Niagara_Ret Niagara::receive_raw(str* source, Niagara_Control* control_output, str* message_output) {
  //If the identifier is empty and not yet initialized then return an error
  if(Niagara::identifier.length() == 0)
    return NIAGARA_NO_IDENTIFIER;

  str receive_output;
  int status = Niagara::lora->receive((uint8_t*)receive_output.c_str(), 0);
  if(status == RADIOLIB_ERR_RX_TIMEOUT) return NIAGARA_TIMEOUT;
  if(status != RADIOLIB_ERR_NONE) return RADIOLIB_ERROR;
  str processed_output[3];
  Niagara::process_message(processed_output, receive_output);
  if(processed_output == nullptr) return NIAGARA_NOT_DESTINATION;
  
  int control_value = processed_output[1].toInt();
 
  *source = processed_output[0];
  *control_output = static_cast<Niagara_Control>(control_value);
  *message_output = processed_output[2];
  return NIAGARA_OK;
}

bool Niagara::process_message(str* output, str message) {
  //If the identifier is empty and not yet initialized then return an error
  if(Niagara::identifier.length() == 0)
    return false;

  //In case the message destination isn't this board then exit
  int separatorIndex = message.indexOf('|');
  if(separatorIndex <= 0) return false;
  str callsign = message.substring(0, separatorIndex - 1);
  //Separate the destination id in source and destination id
  str sourceID, destinationID;
  int identifierSeparator = callsign.indexOf('.');
  if(identifierSeparator <= 0) return false;
  if(callsign.length() <= identifierSeparator) return false;
  sourceID = callsign.substring(0, identifierSeparator - 1);
  destinationID = callsign.substring(identifierSeparator + 1);
  if(!valid_destination(destinationID)) return false;
  if(message.length() <= separatorIndex + 1) return false; 
  int secondSeparatorIndex = message.substring(separatorIndex + 1).indexOf('|');
  if(secondSeparatorIndex <= 0) return false;

  //Separate the str into the control sequence and the message itself
  output[0] = sourceID;
  output[1] = message.substring(separatorIndex + 1, secondSeparatorIndex - 1);
  if(message.length() > secondSeparatorIndex + 1)
    output[2] = message.substring(secondSeparatorIndex + 1);
  else output[2] = "";

  return true;
}

Niagara_Ret Niagara::send_raw(str destination, Niagara_Control control, str message) {
  //If the identifier is empty and not yet initialized then return an error
  if(Niagara::identifier.length() == 0)
    return NIAGARA_NO_IDENTIFIER;

  str formattedMessage = Niagara::format_message(destination, control, message);
  if(formattedMessage.length() == 0) return NIAGARA_INVALID_DATA;
  int status = Niagara::lora->transmit((uint8_t*)formattedMessage.c_str(), 0);
  if(status == RADIOLIB_ERR_TX_TIMEOUT) return NIAGARA_TIMEOUT;
  if(status != RADIOLIB_ERR_NONE)
    return RADIOLIB_ERROR;
  return NIAGARA_OK;
}

str Niagara::format_message(str destination, Niagara_Control control, str message) {
  if(destination.indexOf('|') >= 0 || destination.indexOf('.') >= 0) return "";
  return (Niagara::identifier + "." + destination + "|" + str(static_cast<int>(control)) + "|" + message);
}

bool Niagara::valid_destination(str destination) {
  //Check for broadcast destination
  if(destination == BROADCAST)
    return true;

  //Check if the two strings match in size, if they don't then skip
  if(Niagara::identifier.length() != destination.length()) return false;

  #if defined(ARDUINO)
  for (size_t i = 0; i < Niagara::identifier.length(); ++i) {
      if (tolower(Niagara::identifier.charAt(i)) != tolower(destination.charAt(i))) return false;
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
  //The identifier should be between 6 and 12 characters long
  size_t len = identifier.length();
  if (len < 6 || len > 12) return false;
  if(identifier == BROADCAST) return false;

  //Check if the identifier contains only alphanumeric values
  #if defined(ARDUINO)
  for (int i = 0; i < len; ++i) {
      char c = identifier.charAt(i);
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
