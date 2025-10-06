#include "niagara.h"


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

int NiagaraPi::receive(std::string* source, std::string* control_output, std::string* message_output) {
  std::string receive_output;
  int status = NiagaraPi::lora->receive(receive_output);
  if(status != RADIOLIB_ERR_NONE) return status;
  std::string* processed_output = NiagaraPi::process_message(receive_output);
  if(processed_output == nullptr) return NIAGARA_NOT_DESTINATION;
  if(processed_output[1] != CONTROL_REQUEST_DATA && processed_output[1] != CONTROL_RESPONSE && processed_output[1] != CONTROL_PING)
    return NIAGARA_INVALID_DATA;
  *source = processed_output[0];
  *control_output = processed_output[1];
  *message_output = processed_output[2];
  return RADIOLIB_ERR_NONE;
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
  if(destinationID != identifier) return nullptr;
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

int NiagaraPi::send(std::string destination, std::string control, std::string message) {
  std::string formattedMessage = NiagaraPi::format_message(destination, control, message);
  if(formattedMessage.length() == 0) return NIAGARA_INVALID_DATA;
  int status = NiagaraPi::lora->transmit(formattedMessage);
  return status;
}

std::string NiagaraPi::format_message(std::string destination, std::string control, std::string message) {
  if(destination.indexOf('|') >= 0 || destination.indexOf('.') >= 0 || control.indexOf('|') >= 0) return "";
  //Check the control std::string
  if(control != CONTROL_REQUEST_DATA && control != CONTROL_RESPONSE && control != CONTROL_PING)
    return "";
  return (NiagaraPi::identifier + "." + destination + "|" + control + "|" + message);
}
