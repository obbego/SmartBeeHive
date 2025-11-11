#include "niagara.h"

int main(void) {
	Niagara device;
	if(!device.set_identifier("RASPI")) {
		fprintf(stderr, "Error while setting identifier.\n");
		return 1;
	}
	
	Niagara_Ret error;

	error = device.send("ESP32", "Hello!");
	if(error != NIAGARA_OK) {
		fprintf(stderr, "Error while sending data: %d\n", static_cast<int>(error));
		return 2;
	}

	str receive;
	str source;
	error = device.receive(&source, &receive);
	if(error != NIAGARA_OK) {
		fprintf(stderr, "Error while receiving data: %d\n", static_cast<int>(error));
		return 3;
	}

	printf("Received: %s\n", receive);

	return 0;
}
