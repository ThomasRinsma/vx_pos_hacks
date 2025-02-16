// arm-none-eabi-gcc -g -O0 -mlittle-endian shellcode_thumb.c -o shellcode_thumb -O0 -fPIE -nostdlib -mthumb -ffreestanding -march=armv6 -fomit-frame-pointer -Wl,-Ttext=0x18d9e2

//    (addr is offset to prompt_mode() minus the space for the constants)
// arm-none-eabi-objdump -d shellcode_thumb
// arm-none-eabi-objcopy -O binary --only-section=.text shellcode_thumb shellcode_thumb_text

static const char buf1[];
static const char buf2[];
static const char buf3[];

__attribute__ ((aligned (2)))
void hax() {
	int i;
	char *dest;

	dest = (char *)0x400001b4;
	for (int i = 0; i < 7*4; ++i) {
		dest[i] = buf1[i];
	}

	dest = (char *)0x40500000;
	for (int i = 0; i < 28*4; ++i) {
		dest[i] = buf2[i];
	}

	dest = (char *)0x40510000;
	for (int i = 0; i < 5*4; ++i) {
		dest[i] = buf3[i];
	}
}

void _start() {
	hax();
}

__attribute__((section(".text")))
static const char buf1[] = {
	"\x00\x00\x00\x00"  // 0x400001b4: NOP
	"\xe5\x9f\x00\x0c"  // 0x400001b8: ldr  r0, [0x400001cc]
	"\xe1\x2f\xff\x30"  // 0x400001bc: blx r0
	"\x00\x00\x00\x00"  // 0x400001c0: NOP
	"\xe3\xa0\x01\x01"  // 0x400001c4: mov r0,#0x40000000
	"\xe1\x2f\xff\x30"  // 0x400001c8: blx r0
	"\x40\x50\x00\x00"  // 0x400001cc: 0x40500000
};

// I believe this is modified assembly output from shellcode.c but it's been a while.. 
__attribute__((section(".text")))
static const char buf2[] = {
	"\xe2\x4d\xd0\x10"  // sub	sp, sp, #16
	"\xe5\x9f\x30\x5c"  // ldr	r3, [pc, #92]	; 8068 <hax+0x68>
	"\xe5\x8d\x30\x08"  // str	r3, [sp, #8]
	"\xe5\x9f\x30\x58"  // ldr	r3, [pc, #88]	; 806c <hax+0x6c>
	"\xe5\x8d\x30\x04"  // str	r3, [sp, #4]
	"\xe3\xa0\x30\x00"  // mov	r3, #0
	"\xe5\x8d\x30\x0c"  // str	r3, [sp, #12]
	"\xea\x00\x00\x0a"  // b	804c <hax+0x4c>
	"\xe5\x9d\x30\x0c"  // ldr	r3, [sp, #12]
	"\xe5\x9d\x20\x04"  // ldr	r2, [sp, #4]
	"\xe0\x82\x20\x03"  // add	r2, r2, r3
	"\xe5\x9d\x30\x0c"  // ldr	r3, [sp, #12]
	"\xe5\x9d\x10\x08"  // ldr	r1, [sp, #8]
	"\xe0\x81\x30\x03"  // add	r3, r1, r3
	"\xe5\xd2\x20\x00"  // ldrb	r2, [r2]
	"\xe5\xc3\x20\x00"  // strb	r2, [r3]
	"\xe5\x9d\x30\x0c"  // ldr	r3, [sp, #12]
	"\xe2\x83\x30\x01"  // add	r3, r3, #1
	"\xe5\x8d\x30\x0c"  // str	r3, [sp, #12]
	"\xe5\x9d\x30\x0c"  // ldr	r3, [sp, #12]
	"\xe3\x53\x00\x14"  // cmp	r3, #0x14     <-- size of patch - 1
	"\xda\xff\xff\xf1"  // ble	8020 <hax+0x20>
	"\xe1\xa0\x00\x00"  // nop			; (mov r0, r0)
	"\xe1\xa0\x00\x00"  // nop			; (mov r0, r0)
	"\xe2\x8d\xd0\x10"  // add	sp, sp, #16
	"\xe1\x2f\xff\x1e"  // bx	lr
	// "\x40\x00\x75\x34"  // .word	0x40007534 <-- where to copy to (OS QT000500)
	"\x40\x00\x4b\x70"  // .word	0x40004b70 <-- where to copy to (OS QTE50301)

	"\x40\x51\x00\x00"  // .word	0x40510000 <-- where to copy from
};
__attribute__((section(".text")))
static const char buf3[] = {
	"\xe2\x8f\x00\x00"  // adr r0,+0x8 (0x4000753c)
	// "\xeb\x00\x1d\xe6"  // bl  auth_and_generate_sig_file (@ 0x4000ecd8) // OS QT000500
	"\xeb\x00\x1a\x45"  // bl  auth_and_generate_sig_file (@ 0x4000b490) // OS QTE50301
	// general
	"\x49\x3a\x31\x2f"  // ds  "I:1/"
	"\x48\x2e\x4f\x55"  // ds  "H.OU"
	"\x54\x00\x00\x00"  // ds  "T\0\0\0"
};
