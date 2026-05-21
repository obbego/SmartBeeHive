#include <stdlib.h>
#include <heltec_unofficial.h>
#include <niagara.h>
#include <measure.h>
#include "esp_sleep.h"

/* constants declaration for 
customized parameters of the code */
#define DEVICE_IDENTIFIER "Arnia0"
#define DESTINATION_IDENTIFIER "LoRaREC" 
/* define timing intervals 
to gather telemetries. 
The intervals are expressed in hours */
#define TEMPERATURE_INTERVAL 1
#define HUMIDITY_INTERVAL 1
#define WEIGHT_INTERVAL 1
#define NOISEINTENSITY_INTERVAL 1
#define NOISEFREQUENCY_INTERVAL 1
/* define after how many hours is 
necessary to send telemetries */
#define SENDING_INTERVAL 8
#define ACTIVITY_INTERVAL 1 

/* ===== PIN ===== */
#define DHTPIN   7
#define DHTTYPE  DHT11

#define HX_DT    5
#define HX_SCK   4
#define MIC_PIN  6   // ADC1 ESP32

/* ===== FFT ===== */
#define SAMPLES 512
#define SAMPLING_FREQUENCY 8000

/* ===== OGGETTI ===== */
DHT dht(DHTPIN, DHTTYPE);
HX711 scale;

double vReal[SAMPLES];
double vImag[SAMPLES];
ArduinoFFT<double> FFT(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);

unsigned long samplingPeriodUs;

/* ===== CALIBRAZIONE BILANCIA ===== */
float calibration_factor = -7050.0;

/* declare hour conter to understand whether it's time to
send measures */
RTC_DATA_ATTR hour_counter = 0;

RTC_DATA_ATTR log_initialised = false;

/* global variables */
str destination = str(DESTINATION_IDENTIFIER);

/* define the measures available 
on the board */
char TELEMETRYTYPE[][30] = {
  {"temperature"},
  {"humidity"},
  {"weight"},
  {"noise_intensity"},
  {"noise_frequency"}
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

  Measure telemetry = Measure(TELEMETRYTYPE[number], value);
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

/* function to put the ESP32 device
into deep sleep for the hours put in input */
void deep_sleep_hours(int hours) {
  esp_sleep_enable_timer_wakeup(
      (uint64_t)hours * 3600ULL * 1000000ULL
  );

  esp_deep_sleep_start();
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
  hour_counter++; // start updating the hour counter

  /* control if the hour counter corresponds
  to the time of getting telemetries */
  if(hour_counter == 1 || hour_counter % TEMPERATURE_INTERVAL == 0){
    Serial.println("Start recording temperature"); 
    // function to get the temperature
    // function to save into file
  }
  if(hour_counter == 1 || hour_counter % HUMIDITY_INTERVAL == 0){
    Serial.println("Start recording humidity"); 
    // function to get the humidity
    // function to save into file
  }
  if(hour_counter == 1 || hour_counter % WEIGHT_INTERVAL == 0){
    Serial.println("Start recording weight"); 
    // function to get the weight
    // function to save into file
  }
  if(hour_counter == 1 || hour_counter % NOISEINTENSITY_INTERVAL == 0){
    Serial.println("Start recording noise intensity"); 
    // function to get the noise intensity
    // function to save into file
  }
  if(hour_counter == 1 || hour_counter % NOISEFREQUENCY_INTERVAL == 0){
    Serial.println("Start recording noise frequency"); 
    // function to get the noise frequency
    // function to save into file
  }

  /* if it's time to send data send all
  and reset file memory */
  if(hour_counter % SENDING_INTERVAL == 0){
    Serial.println("Start sending data to the receiver");
    send_telemetries_to_gateway(); // send telemetries
    // function to erase file memory
    hour_counter = 0;
  }

  deep_sleep_hours(ACTIVITY_INTERVAL); // put the device into deep sleep  
}
