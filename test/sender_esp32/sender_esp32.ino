#include <stdlib.h>
#include <heltec_unofficial.h>
#include <niagara.h>
#include <measure.h>

bool log_initialised = false;

/* define the pinout of the board */
char TELEMETRYTYPE[][50] = {
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
Measure getInstantTelemetry(){
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

  Measure telemetry = Measure(TELEMETRYTYPE[number], value, millis());
  return telemetry;
}

Niagara *device;

void setup() {
    // Inizializza la seriale (chiamando l'handler o direttamente)
    Serial.begin(115200);
    log_initialised = true;

    device = new Niagara();
    if(!device->set_identifier("Arnia0")) {
        log_handler_serial("Errore: impossibile impostare l'identificatore.\n");
        while(true); 
    }

    randomSeed(esp_random()); // initialize the random generator from casual values
    
    Serial.println("\n--- ESP32 NIAGARA TEST READY ---");
}

void loop() {
  /* START TRANSMISSION MODE 
    The code will send different measures
    generated randmoly just to test fragments,
    the telemetry sent and the whole environment
    connected to ThingsBoard  
  */
  str destination = str("LoRaREC");
  Measure telemetry = getInstantTelemetry(); 
  str input_str_converted = telemetry.toJSON();

  Serial.println("=== AVVIO TRASMISSIONE ===");
  Niagara_Ret error = device->send(destination, input_str_converted);
  
  if (error == NIAGARA_OK) {
      Serial.println("-> Trasmissione completata con successo.");
  } else {
      Serial.print("-> Errore durante la trasmissione. Codice: ");
      Serial.println(static_cast<int>(error));
  }

  delay(2000);
}
