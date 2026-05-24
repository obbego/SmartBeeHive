#include <stdlib.h>
#include <heltec_unofficial.h>
#include "esp_sleep.h"
#include <HX711.h>
#include <arduinoFFT.h>
/* libraries for saving into file */
#include <FS.h>
#include <LittleFS.h>

#include <niagara.h> // library to handle transmission
#include <niagara_measure.h> // library to handle measures 

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
#define NOISE_INTERVAL 1
/* define after how many hours is 
necessary to send telemetries */
#define SENDING_INTERVAL 8
#define ACTIVITY_INTERVAL 1 

/* ===== PINOUT ===== */
#define DHTPIN   7
#define DHTTYPE  DHT11

#define HX_DT    5
#define HX_SCK   4
#define MIC_PIN  6   // ADC1 ESP32

/* ===== FFT (Frequency)===== */
#define SAMPLES 512
#define SAMPLING_FREQUENCY 8000

/* ===== Objects for the sensors ===== */
DHT dht(DHTPIN, DHTTYPE);
HX711 scale;

double vReal[SAMPLES];
double vImag[SAMPLES];
ArduinoFFT<double> FFT(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);

unsigned long samplingPeriodUs;

/* ===== Scale calibration ===== */
float calibration_factor = -7050.0;

/* declare hour conter to understand whether it's time to
send measures */
RTC_DATA_ATTR int hour_counter = 0;
/* variable to save the last timestamp after 
the reset of millis() counter due to board deep sleep */
RTC_DATA_ATTR unsigned long saved_timestamp = 0;

RTC_DATA_ATTR bool log_initialised = false;

/* global variables */
str destination = str(DESTINATION_IDENTIFIER);
Niagara *device;

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

/* local file management */
#define FILE_PATH "./telemetries.csv"


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

/**
 * Function which is accounted to send telemetries
 * to the receiver in order to get eventually the
 * IOT Platform established. 
 * The connection is based on LoRa protocol.
 * @return void
 * @author Daniele Chiarion
 */
void send_telemetries_to_gateway(){
  /* instantiate objects of the library
  niagara defined to handle measures */
  NiagaraSender niagaraSender = NiagaraSender(device);

  /* instantiate file and specify
  mode and controls */
  File file = LittleFS.open(FILE_PATH, FILE_READ);
  
  if (!file) {
    Serial.println("Impossibile aprire il file in scrittura");
    return;
  }

  /* declare variables and cycle
  every row to get the data */
  char telemetry_type[30];
  float telemetry_value;
  unsigned long ms;
  
  while(file.available()){
    String row = file.readStringUntil('\n'); // read row
    row.trim(); // trim row

    int fields = sscanf(row.c_str(), "%[^,],%f,%lu", telemetry_type, &telemetry_value, &ms);
    NiagaraMeasure measure(telemetry_type, telemetry_value, ms); // create measure
    niagaraSender.add_measure(measure); // add measure ti the niagara sender
  }

  Serial.println("=== AVVIO TRASMISSIONE ===");
  int error = niagaraSender.send(DESTINATION_IDENTIFIER, saved_timestamp + millis()); // function that converts in json and sends to the destination
  
  if (error == 0) { // it menas NIAGARA_OK code
      Serial.println("-> Trasmissione completata con successo.");
  } else {
      Serial.print("-> Errore durante la trasmissione. Codice: ");
      Serial.println(error);
  }
}

/**
 * Function to put the ESP32 board in
 * deep sleep mode in order to save energy. 
 * This function is recommended only for this specific type of board.
 * @param hours amount of hours to make the board sleep
 * @return void
 */
void deep_sleep_hours(int hours) {
  /* before putting the ESP32 into deep sleep 
  saved the last timestamp in addition to the 
  time provided to sleep in hours */
  saved_timestamp += millis() + hours * 3600000;
  esp_sleep_enable_timer_wakeup(
      (uint64_t)hours * 3600ULL * 1000000ULL
  );

  esp_deep_sleep_start();
}


/* === SENSORS FUNCTIONS === */
/**
 * Function to get the temperature from 
 * the sensor of the beehive
 * @return temperature in °C
 */
float get_temperature(){
  return dht.readTemperature();
}

/**
 * Function to get the humidity from 
 * the sensor of the beehive
 * @return humidity in %C
 */
float get_humidity(){
  return dht.readHumidity();
}

/**
 * Function to get the weight of the beehive
 * inside portion that is over the scale. 
 * @return weight in kg
 */
float get_weight(){
  float peso = -1000; // initialize to possible error value
  if (scale.is_ready()) {
    peso = scale.get_units(10);
    if (abs(peso) < 0.02) peso = 0;
  }

  return peso;
}

/**
 * Function to get data from the microphone inside 
 * of the beehive. 
 * In this version is provided the noise frequency peak and
 * the noise intensity based on the range provided by the
 * scale of the ESP32 values.
 * @param *rms pointer to the noise intensity
 * @param *peak pointer to the noise frequency
 * @return void
 */
void get_microphone_data(double *rms, double *peak){
  for (int i = 0; i < SAMPLES; i++) {
    unsigned long t0 = micros();
    vReal[i] = analogRead(MIC_PIN);
    vImag[i] = 0;
    while (micros() - t0 < samplingPeriodUs);
  }

  /* Remove DC offset */
  double mean = 0;
  for (int i = 0; i < SAMPLES; i++) mean += vReal[i];
  mean /= SAMPLES;
  for (int i = 0; i < SAMPLES; i++) vReal[i] -= mean;

  /* RMS */
  double sum = 0;
  for (int i = 0; i < SAMPLES; i++) sum += vReal[i] * vReal[i];
  *rms = sqrt(sum / SAMPLES);

  /* FFT */
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();
  *peak = FFT.majorPeak();
}


/* === FILE SAVING FUNCTIONS === */
/**
 * Function to save a single telemetry inside the
 * file provided in the ESP32 following CSV notation.
 * The telemetry will be appended with the rest of the 
 * telemetries recorded before. 
 * @param telemetry_name name of the telemetry
 * @param telemetry_value value of the telemetry
 * @param ms millis recorded from the board when the telemetry was gathered
 * @return void
 * @author Daniele Chiarion
 */
void save_telemetry_into_file(char telemetry_name[], float telemetry_value, unsigned long ms){
  /* open file and specify the mode */
  File file = LittleFS.open(FILE_PATH, FILE_APPEND);
  
  if (!file) {
    Serial.println("Impossibile aprire il file in scrittura");
    return;
  }

  file.printf("%s,%f,%lu\n", telemetry_name, telemetry_value, ms);
  file.close();

  Serial.printf("Telemetria %s %.2f %lu salvata su file!\n", telemetry_name, telemetry_value, ms);
}

/**
 * Function to clear the file inside the ESP32 board.
 * @return void
 * @author Daniele Chiarion
 */
void clear_file(){
  /* open file and specify the mode */
  File file = LittleFS.open(FILE_PATH, FILE_WRITE);
  
  if (!file) {
    Serial.println("Impossibile aprire il file in scrittura");
    return;
  }

  /* there is no need to post something on the file,
  because the write mode has already cleared the content 
  of the file, so just close it. */
  file.close();
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

    /* Setup DHT sensor */
    dht.begin();
    Serial.println("Sensore DHT correttanmente impostato");
  
    /* HX711 scale setup and calibration */
    scale.begin(HX_DT, HX_SCK);
    scale.set_scale(calibration_factor);
    /* tare the scale only at the first setup of the code, 
    otherwise the telemetry will be compromise
    after the first deep sleep of the ESP32 */
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER) {
      scale.tare();
    }
    Serial.println("Bilancia correttamente impostata");
  
    /* ADC noise sensor setup */
    analogReadResolution(12);
    analogSetPinAttenuation(MIC_PIN, ADC_11db);
  
    samplingPeriodUs = 1000000 / SAMPLING_FREQUENCY;

    /* setup of the file where to save 
    the data inside the ESP32 waiting for transmission */
    if (!LittleFS.begin(true)) {
      Serial.println("Errore durante l'avvio di LittleFS");
      return;
    }
    Serial.println("LittleFS avviato correttamente!");
    
    Serial.println("\n--- ESP32 NIAGARA TEST READY ---");
}

void loop() {
  hour_counter++; // start updating the hour counter

  /* control if the hour counter corresponds
  to the time of getting telemetries */
  if(hour_counter == 1 || hour_counter % TEMPERATURE_INTERVAL == 0){
    Serial.println("Start recording temperature"); 
    float temperature = get_temperature();
    save_telemetry_into_file(TELEMETRYTYPE[0], temperature, saved_timestamp + millis());
  }
  if(hour_counter == 1 || hour_counter % HUMIDITY_INTERVAL == 0){
    Serial.println("Start recording humidity"); 
    float humidity = get_humidity();
    save_telemetry_into_file(TELEMETRYTYPE[1], humidity, saved_timestamp + millis());
  }
  if(hour_counter == 1 || hour_counter % WEIGHT_INTERVAL == 0){
    Serial.println("Start recording weight"); 
    float weight = get_weight();
    save_telemetry_into_file(TELEMETRYTYPE[2], weight, saved_timestamp + millis());
  }
  if(hour_counter == 1 || hour_counter % NOISE_INTERVAL == 0){
    Serial.println("Start recording noise"); 
    /* every operation has to be doubled here,
    because the telemetries gathered are both of 
    noise intensity and noise frequenct */
    double noise_intensity, noise_frequency;
    get_microphone_data(&noise_intensity, &noise_frequency);
    save_telemetry_into_file(TELEMETRYTYPE[3], noise_intensity, saved_timestamp + millis());
    save_telemetry_into_file(TELEMETRYTYPE[4], noise_frequency, saved_timestamp + millis());
  }

  /* if it's time to send data send all
  and reset file memory */
  if(hour_counter % SENDING_INTERVAL == 0){
    Serial.println("Start sending data to the receiver");
    send_telemetries_to_gateway(); // send telemetries
    clear_file(); // clear memory before restarting
    hour_counter = 0;
  }

  deep_sleep_hours(ACTIVITY_INTERVAL); // put the device into deep sleep  
}
