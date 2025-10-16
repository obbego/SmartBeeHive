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

Niagara::Niagara(str _identifier, bool log = true) {
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
  *Niagara::lora = radio_return.value();

  //Save identifier
  Niagara::identifier = _identifier;
}

/* If the boolean for the log variable isn't defined, just set it to true */
Niagara::Niagara(str _identifier) : Niagara(_identifier, true) {}

Niagara_Ret Niagara::receive(str* output, str* source = nullptr) {
  //Contains the ID of the first device whose first SYN was received
  str device;
  //Saves the received control signals
  Niagara_Control control;
  //Saves the output of the receivings
  str output;
  //Contains the status of the operations
  Niagara_Ret status;
  //Amount of retransmissions done during the handshake
  int retransmission_counter = 0;

  //Timer to check for retransmissions
  Timer retransmission_timer;

  //Wait until a receive has completed
  while(true) {
    //Try to receive data
    status = Niagara::receive_raw(&source, &control, &output)
    //If it timed out, then retry
    if(status == NIAGARA_TIMEOUT)
      continue;
    //In case an error occurred then return an error
    if(status != NIAGARA_OK)
      return NIAGARA_RECEIVE_ERROR;
    
    //If not a syn req, skip
    if(control != HANDSHAKE_SYN)
      continue;
  }

  //Compute hashed data for error check
  str hashed_data = crc32(output);
  
  //While cycle for retransmissions
  retransmission_timer.start();
  //Set to false when a retransmission is not necessary
  bool retransmit = true;
  while(retransmission_counter < NIAGARA_RETRANSMISSIONS) {
    if(retransmit) {
      retransmit = true;
      //Send the first acknowledgement
      if(Niagara::send_raw(device, HANDSHAKE_ACK, hashed_data) != NIAGARA_OK)
        return NIAGARA_SEND_ERROR;
    }
      
    //Try to receive new data, check the source
    str source;
    status Niagara::receive_raw(&source, &control, &output);
    if(status == NIAGARA_TIMEOUT) {
      retransmission_counter++;
      continue;
    }
    if(status != NIAGARA_OK && status != NIAGARA_TIMEOUT) {
      if(retransmission_timer.elapsed() > NIAGARA_TIMEOUT) {
        retransmission_timer.start();
        retransmission_counter++;
        continue;
      }

      retransmit = false;
      continue;
    }
    
    //If the source isn't the device which is handshaking or the control message isn't a ping then ignore
    if(source != device)
      continue;

    if(control == HANDSHAKE_ERROR)
      break; //Reperform the receiving and increase the retransmission count

    //If the control message is different then ignore the message
    if(control != HANDSHAKE_ACK)
      continue;
  }

  //If the max retransmission amount was reached then ignore
  if(retransmission_counter == NIAGARA_RETRANSMISSIONS)
    return NIAGARA_TIMEOUT;
  
  return NIAGARA_OK;
}

Niagara_Ret Niagara::send(str identifier, str message) {
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
      if(Niagara::send_raw(identifier, HANDSHAKE_SYN, message) != NIAGARA_OK)
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
    if(source != identifier || (control_msg != HANDSHAKE_ACK && control_msg != HANDSHAKE_ERROR)) {
      if(retransmission_timer.elapsed() > NIAGARA_TIMEOUT)
      {
        retransmission_timer.start();
        retransmission_counter++;
        continue;
      }

      retransmit = false;
      continue;
    }
  }

  if(retransmission_counter == NIAGARA_RETRANSMISSIONS)
    return NIAGARA_TIMEOUT;

  //Send the last acknowledgement and exit
  if(Niagara::send_raw(device, HANDSHAKE_ACK, "") != NIAGARA_OK)
    return NIAGARA_SEND_ERROR;

  return NIAGARA_OK;
}

Niagara_Ret Niagara::receive_raw(str* source, Niagara_Control* control_output, str* message_output, int timeout = 0) {
  str receive_output;
  int status = Niagara::lora->receive(receive_output, timeout);
  if(status == RADIOLIB_ERR_RX_TIMEOUT) return NIAGARA_TIMEOUT;
  if(status != RADIOLIB_ERR_NONE) return RADIOLIB_ERROR;
  str* processed_output = Niagara::process_message(receive_output);
  if(processed_output == nullptr) return NIAGARA_NOT_DESTINATION;
  
  int control_value;
  #if defined(ARDUINO)
  if(!isValidInteger(processed_output[1]))
    return NIAGARA_INVALID_DATA;
  control_value = processed_output[1].toInt();
  if(control_value < 0 || control_value >= END)
    return NIAGARA_INVALID_DATA;
  #else
  try {
    control_value = std::stoi(processed_output[1]);
    if(control_value < 0 || control_value >= END)
      return NIAGARA_INVALID_DATA;
  } catch(...) {
    return NIAGARA_INVALID_DATA;
  }
  #endif
  *source = processed_output[0];
  *control_output = static_cast<Niagara_Control>(control_value);
  *message_output = processed_output[2];
  return NIAGARA_OK;
}

str* Niagara::process_message(str message) {
  //In case the message destination isn't this board then exit
  int separatorIndex = message.indexOf('|');
  if(separatorIndex <= 0) return nullptr;
  str callsign = message.substring(0, separatorIndex - 1);
  //Separate the destination id in source and destination id
  str sourceID, destinationID;
  int identifierSeparator = callsign.indexOf('.');
  if(identifierSeparator <= 0) return nullptr;
  if(callsign.length() <= identifierSeparator) return nullptr;
  sourceID = callsign.substring(0, identifierSeparator - 1);
  destinationID = callsign.substring(identifierSeparator + 1);
  if(destinationID != BROADCAST && destinationID != identifier) return nullptr;
  if(message.length() <= separatorIndex + 1) return nullptr;
  int secondSeparatorIndex = message.substring(separatorIndex + 1).indexOf('|');
  if(secondSeparatorIndex <= 0) return nullptr;

  //Separate the str into the control sequence and the message itself
  str separated[3];
  separated[0] = sourceID;
  separated[1] = message.substring(separatorIndex + 1, secondSeparatorIndex - 1);
  if(message.length() > secondSeparatorIndex + 1)
    separated[2] = message.substring(secondSeparatorIndex + 1);
  else separated[2] = "";

  return separated;
}

Niagara_Ret Niagara::send_raw(str destination, Niagara_Control control, str message) {
  str formattedMessage = Niagara::format_message(destination, control, message);
  if(formattedMessage.length() == 0) return NIAGARA_INVALID_DATA;
  int status = Niagara::lora->transmit(formattedMessage);
  if(status == RADIOLIB_ERR_TX_TIMEOUT) return NIAGARA_TIMEOUT;
  if(status != RADIOLIB_ERR_NONE)
    return RADIOLIB_ERROR;
  return NIAGARA_OK;
}

str Niagara::format_message(str destination, Niagara_Control control, str message) {
  if(destination.indexOf('|') >= 0 || destination.indexOf('.') >= 0 || control.indexOf('|') >= 0) return "";
  return (Niagara::identifier + "." + destination + "|" + static_cast<int>(control) + "|" + message);
}