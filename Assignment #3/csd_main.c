#include <stdio.h>

#define csd_SWITCH_ADDR 0x41210000

// Take input from the switches on the board (Priority: SW7 > .. > SW0)
// return duration value (unit: 100ms)
int get_duration() {
	unsigned char* sw = (unsigned char*)csd_SWITCH_ADDR;

	unsigned char sw_val = *sw;
	unsigned int duration;

	if (sw_val & 0x80) {		// SW7 is in the up position
		duration = 1;			// ON duration is roughly 100ms
	}
	else if (sw_val & 0x40) {	// SW6 is in the up position
		duration = 2;			// ON duration is roughly 200ms
	}
	else if (sw_val & 0x20) {	// SW5 is in the up position
		duration = 3;			// ON duration is roughly 300ms
	}
	else if (sw_val & 0x10) {	// SW4 is in the up position
		duration = 4;			// ON duration is roughly 400ms
	}
	else if (sw_val & 0x8) {	// SW3 is in the up position
		duration = 5;			// ON duration is roughly 500ms
	}
	else if (sw_val & 0x4) {	// SW2 is in the up position
		duration = 6;			// ON duration is roughly 600ms
	}
	else if (sw_val & 0x2) {	// SW1 is in the up position
		duration = 7;			// ON duration is roughly 700ms
	}
	else if (sw_val & 0x1) {	// SW0 is in the up position
		duration = 8;			// ON duration is roughly 800ms
	}
	else {						// Otherwise
		duration = 10;			// ON duration is roughly 1000ms (1s)
	}

	// Zynq-7000 TRM p64, "Each Cortex-A9 CPU can issue two instructions in one cycle and execute them out of order"
	// Zynq-7000 (XC7Z020) - Clock Frequency: 667MHz
	duration *= (unsigned int)66700000;
	return duration;
}
