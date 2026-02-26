#include "niagara.h"

void setup() {
	//Initialise the device.
	Niagara device;
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
		if(error != NIAGARA_OK) {
			device.display_printf("Error while sending data: %d\n", static_cast<int>(error));
			return;
		}
		
		device.display_print("Successful send.\n");
	}

	str receive;
	str source;
	error = device.receive(&source, &receive);
	if(error != NIAGARA_OK) {
		device.display_printf("Error while receiving data: %d\n", static_cast<int>(error));
		return;
	}
	
	device.display_printf("Received: '%s'\n", receive);
}
