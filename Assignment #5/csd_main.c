#include <stdio.h>
#include "platform.h"
#include "sleep.h"
#include <time.h>

// Every second, the time information (hour:minute:second) should be sent to the UART terminal on PC.
// - The time starts from 00:00:00
// - Use some special ASCII characters to return back the cursor to the first column, when you display a new time.
//   (That is, you don't want to use the next line to display a new time)
int show_time() {
	static int sec = 0;
	int h, m, s;

	sec %= 60*60*24;	// time: 00:00:00 ~ 23:59:59

	h = sec / 3600;
	m = (sec % 3600) / 60;
	s = (sec % 3600) % 60;
	printf("%02d:%02d:%02d\r", h, m, s);	// "\r" to return back the cursor to the first column
	sec++;

	return 0;
}


// Setup UART & Turn off output buffering of stdout
int setup() {
    init_platform();				// UART setup

    // https://stackoverflow.com/questions/1716296/why-does-printf-not-flush-after-the-call-unless-a-newline-is-in-the-format-strin
    setvbuf(stdout, 0, _IONBF, 0);	// Turn off output buffering of stdout

    return 0;
}


int clear() {
	cleanup_platform();

	return 0;
}
