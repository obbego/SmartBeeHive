#include "niagara.h"
#include "Timer.h"

std::optional<SX1262> NiagaraPi::init_radio() {
  // now we can create the radio module
  SX1262 radio = new Module(hal, 21, 16, 18, 20 /*The BUSY pin of the module MUST be specified, otherwise error -2 is thrown*/);
  
  if(NiagaraPi::display_log) fprintf(stderr, "[SX1262] Initializing... ");
  /* The module is being initialized with all the default begin() settings
   * The only settings changed are the following:
   * - The frequency, according to the EU868 standard must be 868MHz, the default frequency is 434MHz
   * - The TCXO voltage, which is the crystal which is powering the clock, since this module is not using TCXO, 
   *   not specifying that will cause error -707 to be thrown, so we need to specify its voltage to be 0
   */
  int state = radio.begin(868.0 /*EU868 frequency*/, 125.0, 9, 7, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 10, 8, 0 /*This is not the default value*/, false);
  if(state != RADIOLIB_ERR_NONE) {
    if(NiagaraPi::display_log) fprintf(stderr, "Initialization Failed!\nError code: %d\n", state);
    return std::nullopt;
  }
  if(NiagaraPi::display_log) fprintf(stderr, "Initialization successful!\n");
  return radio;
}

NiagaraPi::NiagaraPi(std::string _identifier, bool log = true) {
  //Save the log flag the user specified
  display_log = log;
  
  // use SPI channel 1, because on Waveshare LoRaWAN Hat,
  // the SX1261 CS is connected to CE1
  NiagaraPi::hal = new PiHal(1);

  //Initialize the radio module
  std::optional<SX1262> radio_return = init_radio();
  if(!radio_return.has_value()) return;
  *NiagaraPi::lora = radio_return.value();

  //Save identifier
  NiagaraPi::identifier = _identifier;
}

NiagaraPi::NiagaraPi(std::string identifier) {
  //Save the log flag the user specified
  display_log = true;
  
  // use SPI channel 1, because on Waveshare LoRaWAN Hat,
  // the SX1261 CS is connected to CE1
  NiagaraPi::hal = new PiHal(1);

  //Initialize the radio module
  std::optional<SX1262> radio_return = init_radio();
  if(!radio_return.has_value()) return;
  *NiagaraPi::lora = radio_return.value();

  //Save identifier
  NiagaraPi::identifier = _identifier;
}

Niagara_Ret NiagaraPi::listen() {
  //Contains the ID of the first device whose first SYN was received
  std::string device;
  //Saves the received control signals
  Niagara_Control control;
  //Saves the output of the receivings, unused
  std::string output;
  //Contains the status of the operations
  Niagara_Ret status;
  //Amount of retransmissions done during the handshake
  int retransmission_counter = 0;

  //Timer to check for retransmissions
  Timer retransmission_timer;

  //Wait until a receive has completed
  while(true) {
    //Try to receive data
    status = NiagaraPi::receive(&source, &control, &output)
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
  
  //While cycle for retransmissions
  retransmission_timer.start();
  //Set to false when a retransmission is not necessary
  bool retransmit = true;
  while(retransmission_counter < NIAGARA_RETRANSMISSIONS) {
    if(retransmit) {
      retransmit = true;
      //Send the first acknowledgement
      if(NiagaraPi::send(device, HANDSHAKE_ACK, "") != NIAGARA_OK)
        return NIAGARA_SEND_ERROR;
    }
      
    //Try to receive new data, check the source
    std::string source;
    status NiagaraPi::receive(&source, &control, &output);
    if(status == NIAGARA_TIMEOUT) {
      retransmission_counter++;
      continue;
    }
    if(status != NIAGARA_OK && status != NIAGARA_TIMEOUT) {
      if(retransmission_timer.elapsed() > 10000) {
        retransmission_timer.start();
        retransmission_counter++;
        continue;
      }

      retransmit = false;
      continue;
    }
    
    //If the source isn't the device which is handshaking or the control message isn't a ping then ignore
    if(source != device || control != HANDSHAKE_ACK)
      continue;
  }

  //If the max retransmission amount was reached then ignore
  if(retransmission_counter == NIAGARA_RETRANSMISSIONS)
    return NIAGARA_TIMEOUT;
  
  return NIAGARA_OK;
}

Niagara_Ret NiagaraPi::connect(std::string identifier) {
  //Source of the receivings
  std::string source;
  //Received control message
  Niagara_Control control_msg;
  //Buffer to save the payload
  std::string message;
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
      if(NiagaraPi::send(identifier, HANDSHAKE_SYN, "") != NIAGARA_OK)
        return NIAGARA_SEND_ERROR;
    }

    //Wait for acknowledgement
    status = NiagaraPi::receive(&source, &control_msg, &message);
    if(status == NIAGARA_TIMEOUT) {
      retransmission_counter++;
      continue;
    }
    if(status != NIAGARA_OK)
      return NIAGARA_RECEIVE_ERROR;

    //Check that source and received parameters match with an acknowledgement
    if(source != identifier || control_msg != HANDSHAKE_ACK) {
      if(retransmission_timer.elapsed() > 10000)
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
  if(NiagaraPi::send(device, HANDSHAKE_ACK, "") != NIAGARA_OK)
    return NIAGARA_SEND_ERROR;

  return NIAGARA_OK;
}

Niagara_Ret NiagaraPi::receive(std::string* source, Niagara_Control* control_output, std::string* message_output, int timeout = 0) {
  std::string receive_output;
  int status = NiagaraPi::lora->receive(receive_output, timeout);
  if(status == RADIOLIB_ERR_RX_TIMEOUT) return NIAGARA_TIMEOUT;
  if(status != RADIOLIB_ERR_NONE) return RADIOLIB_ERROR;
  std::string* processed_output = NiagaraPi::process_message(receive_output);
  if(processed_output == nullptr) return NIAGARA_NOT_DESTINATION;
  
  int control_value;
  try {
    control_value = std::stoi(processed_output[1]);
    if(control_value < 0 || control_value >= END)
      return NIAGARA_INVALID_DATA;
  } catch(...) {
    return NIAGARA_INVALID_DATA;
  }
  *source = processed_output[0];
  *control_output = static_cast<Niagara_Control>(control_value);
  *message_output = processed_output[2];
  return NIAGARA_OK;
}

std::string* NiagaraPi::process_message(std::string message) {
  //In case the message destination isn't this board then exit
  int separatorIndex = message.indexOf('|');
  if(separatorIndex <= 0) return nullptr;
  std::string callsign = message.substring(0, separatorIndex - 1);
  //Separate the destination id in source and destination id
  std::string sourceID, destinationID;
  int identifierSeparator = callsign.indexOf('.');
  if(identifierSeparator <= 0) return nullptr;
  if(callsign.length() <= identifierSeparator) return nullptr;
  sourceID = callsign.substring(0, identifierSeparator - 1);
  destinationID = callsign.substring(identifierSeparator + 1);
  if(destinationID != BROADCAST && destinationID != identifier) return nullptr;
  if(message.length() <= separatorIndex + 1) return nullptr;
  int secondSeparatorIndex = message.substring(separatorIndex + 1).indexOf('|');
  if(secondSeparatorIndex <= 0) return nullptr;

  //Separate the std::string into the control sequence and the message itself
  std::string separated[2];
  separated[0] = message.substring(separatorIndex + 1, secondSeparatorIndex - 1);
  if(message.length() > secondSeparatorIndex + 1)
    separated[1] = message.substring(secondSeparatorIndex + 1);
  else separated[1] = "";

  return separated;
}

Niagara_Ret NiagaraPi::send(std::string destination, Niagara_Control control, std::string message) {
  std::string formattedMessage = NiagaraPi::format_message(destination, control, message);
  if(formattedMessage.length() == 0) return NIAGARA_INVALID_DATA;
  int status = NiagaraPi::lora->transmit(formattedMessage);
  if(status == RADIOLIB_ERR_TX_TIMEOUT) return NIAGARA_TIMEOUT;
  if(status != RADIOLIB_ERR_NONE)
    return RADIOLIB_ERROR;
  return NIAGARA_OK;
}

std::string NiagaraPi::format_message(std::string destination, Niagara_Control control, std::string message) {
  if(destination.indexOf('|') >= 0 || destination.indexOf('.') >= 0 || control.indexOf('|') >= 0) return "";
  return (NiagaraPi::identifier + "." + destination + "|" + static_cast<int>(control) + "|" + message);
}
