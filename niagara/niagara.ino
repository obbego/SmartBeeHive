#include "niagara.h"
#include <heltec_unofficial.h>

void log_handler(const char* text) {
	display.print(text);
}

void setup() {
	//Initialise the display and heltec library
	heltec_setup();
	display.init();
	display.setFont(ArialMT_Plain_10);

	//Initialise the device with its log handler.
	Niagara device(log_handler, Niagara_LogLevel::DISPLAY);
	if(!device.set_identifier("ESP32")) {
		device.display_print("Error while setting identifier.\n");
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
	if(millis() - send_timer > send_treshold) {
		send_timer = millis();

		error = device.send("RASPI", "Hello from ESP!");
		if(error != NIAGARA_OK) return;
	}

	str receive;
	str source;
	error = device.receive(&source, &receive);
	if(error != NIAGARA_OK) return;
	
	device.display_printf("Received: '%s'\n", receive);
}
