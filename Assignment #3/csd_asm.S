// ------------------------------------------
//  Author: Yeonseok Jang
//          Computer Science & Engineering
//          College of Informatics, Korea Univ.
//  Date:   March 31, 2022
// ------------------------------------------


#define csd_LED_ADDR 0x41200000

.extern get_duration

.global main
main:
	ldr r4, =csd_LED_ADDR
	mov r5, #0x1			// LD0(0x1)  | LD1(0x2)  | LD2(0x4)  | LD3(0x8)  |
							// LD4(0x10) | LD5(0x20) | LD6(0x40) | LD7(0x80)
	loop:
		bl get_duration		// Check out the switch input at the begging of each iteration
		str r5, [r4]		// Turn on LED

		// https://stackoverflow.com/questions/27510198/how-to-delay-an-arm-cortex-m0-for-n-cycles-without-a-timer
		mov r6, r0
		L:
			subs r6, r6, #1
			bne L

		mov r5, r5, LSL#1	// Next LED
		tst r5, #0x100		// check if r1 is 0x100
		movne r5, #0x1		// if r1 & 0x100 != 0, then r1=1

		b loop				// iterate

