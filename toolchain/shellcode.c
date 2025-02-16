// arm-none-eabi-gcc -g -O0 -mlittle-endian shellcode.c -o shellcode -O0 -fPIE -nostdlib -marm -mno-thumb-interwork -ffreestanding -march=armv6 -fomit-frame-pointer
// arm-none-eabi-objdump -d shellcode
// arm-none-eabi-objcopy -O binary --only-section=.text shellcode shellcode_text
// ^ and then don't forget to swap the instruction bytes

/*
	The plan:

	- in jump_to_os(), we add a `blx 0x40080000` in place of the CRC check / loop.
	- at 0x40080000, we can add as much code as we want, then return
*/

void hax() {
	// Copy N bytes from our buffer to target addr

	#define N 5
	#define TARGET 0x4001B253
	#define SOURCE 0x40510000
	
	char *to   = (char *)TARGET;
	char *from = (char *)SOURCE;
	for (int i = 0; i < N; ++i) {
		to[i] = from[i];
	}
}



void _start() {

	// First call our new function which does the patching
	asm("ldr r0, =0x40080000");
	asm("blx r0");

	// Then jump to entry as usual
	asm("mov r0, #0x40000000");
	asm("blx r0");

}


