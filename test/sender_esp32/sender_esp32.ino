#include <stdlib.h>
#include <heltec_unofficial.h>
#include <niagara.h>
#include <measure.h>

/* constants declaration for 
customized parameters of the code */
#define DEVICE_IDENTIFIER "Arnia0"
#define DESTINATION_IDENTIFIER "LoRaREC" 
/* define timing intervals 
to gather telemetries. 
The intervals are expressed in hours */
#define TEMPERATURE_INTERVAL 1
#define HUMIDITY_INTERVAL 1
#define WEIGHT_INTERVAL 3
#define NOISEINTENSITY_INTERVAL 2
#define NOISEFREQUENCY_INTERVAL 2

bool log_initialised = false;

/* global variables */
str destination = str(DESTINATION_IDENTIFIER);

/* define a struct for each measure and 
it's last time to be recorded, 
using the board clock */
typedef struct{
  char telemetry_name[30];
  unsigned long last_record;
}telemetrytype_t;

/* define the measures available 
on the board */
telemetrytype_t TELEMETRYTYPE[] = {
  {"temperature", 0},
  {"humidity", 0},
  {"weight", 0},
  {"noise_intensity", 0},
  {"noise_frequency", 0}
};
int size = sizeof(TELEMETRYTYPE)/sizeof(TELEMETRYTYPE[0]);

void log_handler_display(const char* text) {
	if(!log_initialised) {
		//Initialise the display and heltec library
		heltec_setup();
		display.init();
		display.setFont(ArialMT_Plain_10);

		// Mark the log as initialised
		log_initialised = true;
	}

	display.print(text);
}

void log_handler_serial(const char* text) {
    Serial.print(text);
}

/* function to generate the measure
and return the corresponding string to send */
Measure get_instant_telemetry(){
  int number = random(0, size); // pick a random telemetry type
  float value = 0.0;

  /* based on the number, 
  select the random interval to generate the telemetry */
  switch(number){
    case 0:
      value = random(0,80) - 20;
      break;
    case 1:
      value = random(0,100);
      break;
    case 2:
      value = random(0,20);
      break;
    case 3:
      value = random(0,120);
      break;
    case 4:
      value = random(0,500);
      break;
    default:
      value = random(0,100);
  }

  Measure telemetry = Measure(TELEMETRYTYPE[number].telemetry_name, value);
  return telemetry;
}

Niagara *device;

void send_telemetries_to_gateway(){
  /* START TRANSMISSION MODE 
    The code will send different measures
    generated randmoly just to test fragments,
    the telemetry sent and the whole environment
    connected to ThingsBoard  
  */

  /* this section will be replaced with the 
  manager of multiple telemetries which will be 
  converted into the JSON format */
  Measure telemetry = get_instant_telemetry(); 
  str input_str_converted = telemetry.toJSON();

  Serial.println("=== AVVIO TRASMISSIONE ===");
  Niagara_Ret error = device->send(destination, input_str_converted);
  
  if (error == NIAGARA_OK) {
      Serial.println("-> Trasmissione completata con successo.");
  } else {
      Serial.print("-> Errore durante la trasmissione. Codice: ");
      Serial.println(static_cast<int>(error));
  }
}

void setup() {
    // Inizializza la seriale (chiamando l'handler o direttamente)
    Serial.begin(115200);
    log_initialised = true;

    device = new Niagara();
    if(!device->set_identifier(DEVICE_IDENTIFIER)) {
        log_handler_serial("Errore: impossibile impostare l'identificatore.\n");
        while(true); 
    }

    randomSeed(esp_random()); // initialize the random generator from casual values
    
    Serial.println("\n--- ESP32 NIAGARA TEST READY ---");
}

void loop() {
  /* variable declaration */
  str payload;

  /* receive the message and evaluate
  whether data has to been sent or not. 
  N.B. This operation is a blocking function which ends 
  after a timeout of few seconds, so it won't block the entire system
  till it gets a valid value. */
  Niagara_Ret receive_output = device->receive(&payload, &destination); // the device to listen is the gateway, so the normal destination

  /* detect the payload and select the
  different requests from the device */
  if(receive_output == NIAGARA_OK){
    if(payload.c_str() == "TELEMETRY_REQUEST"){
      /* normal response to request of the gateway
      to ge the telemetries */
      send_telemetries_to_gateway();
    }else{
      /* report unusual request from the device */
      Serial.printf("Unknown request '%s' coming from device '%s'\n", payload.c_str(), destination.c_str());
    }
  }
  else{
   unsigned long current_time = millis();
   /* control if it's time to gather 
   some telemetries */
   if(TELEMETRYTYPE[0].last_record == 0 || (current_time-TELEMETRYTYPE[0].last_record)/3600000 > TEMPERATURE_INTERVAL){
    Serial.println("Started recording temperature");
    // function to get the temperature
    TELEMETRYTYPE[0].last_record = millis(); // update last record
   } 
   if(TELEMETRYTYPE[1].last_record == 0 || (current_time-TELEMETRYTYPE[1].last_record)/3600000 > HUMIDITY_INTERVAL){
    Serial.println("Started recording humidity");
    // function to get the humidity
    TELEMETRYTYPE[1].last_record = millis(); // update last record
    
   }
   if(TELEMETRYTYPE[2].last_record == 0 || (current_time-TELEMETRYTYPE[2].last_record)/3600000 > WEIGHT_INTERVAL){
    Serial.println("Started recording weight");
    // function to get the weight
    TELEMETRYTYPE[2].last_record = millis(); // update last record
   }
   if(TELEMETRYTYPE[3].last_record == 0 || (current_time-TELEMETRYTYPE[3].last_record)/3600000 > NOISEINTENSITY_INTERVAL){
    Serial.println("Started recording noise intensity");
    // function to get the noise intensity
    TELEMETRYTYPE[3].last_record = millis(); // update last record
   }
   if(TELEMETRYTYPE[4].last_record == 0 || (current_time-TELEMETRYTYPE[4].last_record)/3600000 > NOISEFREQUENCY_INTERVAL){
    Serial.println("Started recording noise frequency");
    // function to get the noise frequency
    TELEMETRYTYPE[4].last_record = millis(); // update last record
   }
  }
}
