#include "niagara.h"
#include "Timer.h"

// Define the callback function for logging
void logger(const char* message) {
	printf(message);
}

int main(void) {
	//Initialise the device
	Niagara device(logger, Niagara_LogLevel::LOG_TERMINAL);
	if(!device.set_identifier("RASPI")) {
		fprintf(stderr, "Error while setting identifier.\n");
		return 1;
	}
	
	//Variable to save error output
	Niagara_Ret error;
	//Send timer
	Timer send_timer;
	const unsigned int send_time = 5000;
	
	send_timer.start();
	
	//Start infinite loop
	for(;;) {
		//If the send timer has elapsed then send a new packet
		if(send_timer.elapsed() > send_time) {
			send_timer.start();
		
			error = device.send("ESP32", "Hello!");
			if(error != NIAGARA_OK) {
				fprintf(stderr, "Error while sending data: %d\n", static_cast<int>(error));
				continue;
			}
		}

		//Try to receive
		str receive;
		str source;
		error = device.receive(&source, &receive);
		if(error != NIAGARA_OK) {
			fprintf(stderr, "Error while receiving data: %d\n", static_cast<int>(error));
			continue;
		}

		printf("Received: %s\n", receive.c_str());
	}
}
