#include "niagara.h"

int main(void) {
	Niagara device();
	device.set_identifier("RASPI");
	device.send("ESP32", "Hello!");

	str receive;
	str source;
	device.receive(&source, &receive);

	printf("Received: %s", receive);

	return 0;
}
