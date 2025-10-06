#include "niagara.h"

string

int main(void) {
	NiagaraPi device = NiagaraPi("RASPI");

	device.send();
}
