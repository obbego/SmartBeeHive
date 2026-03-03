#include <heltec_unofficial.h>
#include "niagara.h"

bool log_initialised = false;

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
	if(!log_initialised) {
		//Initialised the serial
		Serial.begin(115200);

		//Mark the log as initialised
		log_initialised = true;
	}

	Serial.print(text);
}

// Define the device as global pointer
Niagara *device;

void setup() {
	//Initialise the device with its log handler.
	device = new Niagara(log_handler_serial, Niagara_LogLevel::LOG_TERMINAL);
	if(!device->set_identifier("ESP32")) {
		log_handler_serial("Error while setting identifier.\n");
		while(true); //Infinite loop to halt
	}
}

//Send timer management
unsigned long send_timer = millis();
const unsigned long send_treshold = 5000;

//Error output save
Niagara_Ret error;

void loop() {
	//Every send_treshold time send a new packet to RASPI
	//if(millis() - send_timer > send_treshold) {
		//send_timer = millis();

		log_handler_serial("=== SENDING DATA ===");
		error = device->send("RASPI", "Hello from ESP!");
		if(error != NIAGARA_OK) delay(5000);
		else while(true); // If the sent was executed succesfully then halt
	/*
	}

	str receive;
	str source;
	error = device->receive(&source, &receive);
	if(error != NIAGARA_OK) return;
	
	log_handler("Received: '");
	log_handler(receive.c_str());
	log_handler("'\n");
	*/
}
