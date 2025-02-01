#include "lib.h"

// malloc_ptr malloc = (malloc_ptr)0x70008fb0;
// sprintf_ptr sprintf = (sprintf_ptr)0x700002c4; // TODO



 // __attribute__((target("arm")))
// int sprintf(char *, const char *, ...) {
// 	int ret;

// 	asm volatile(
// 		"blx 0x700002c4\n"
// 		"mov %[ret], r0\n"
// 		:
// 			[ret] "=l" (ret)
// 		:
// 		:
// 			"r0", "r1", "r2", "r3", "r4", "r5", "r6", "memory"
// 	);

// 	return ret;
// }


//  // __attribute__((target("arm")))
// char *malloc(size_t size) {
// 	char *ret;

// 	asm volatile(
// 		"blx 0x70008fb0\n"
// 		"mov %[ret], r0\n"
// 		:
// 			[ret] "=l" (ret)
// 		:
// 		:
// 			"r0", "r1", "r2", "r3", "r4", "r5", "r6", "memory"
// 	);

// 	return ret;
// }




int open(const char *path, int flags) {
	int ret;

	asm volatile(
		"mov r0, %[path]\n"
		"mov r1, %[flags]\n"
		"svc 5\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			[path] "l" (path),
			[flags] "l" (flags)
		:
			"r0", "r1", "memory"
	);

	return ret;
}

int close(int fd) {
	int ret;

	asm volatile(
		"mov r0, %[fd]\n"
		"svc 4\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			[fd] "l" (fd)
		:
			"r0", "memory"
	);

	return ret;
}


int write(int fd, const char *buf, unsigned len) {
	int ret;

	asm volatile(
		"mov r0, %[fd]\n"
		"mov r1, %[buf]\n"
		"mov r2, %[len]\n"
		"svc 0\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			[fd] "l" (fd),
			[buf] "l" (buf),
			[len] "l" (len)
		:
			"r0", "r1", "r2", "memory"
	);

	return ret;

}

int read(int fd, char *buf, unsigned len) {
	int ret;

	asm volatile(
		"mov r0, %[fd]\n"
		"mov r1, %[buf]\n"
		"mov r2, %[len]\n"
		"svc 1\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			[fd] "l" (fd),
			[buf] "l" (buf),
			[len] "l" (len)
		:
			"r0", "r1", "r2", "memory"
	);

	return ret;

}


int wait_event(){
	int ret;

	asm volatile(
		"mov r0, #0\n"
		"svc 10\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			/* no inputs */
		:
			"r0", "memory"
	);

	return ret;
}


int clear_screen() {
	int ret;

	asm volatile(
		"mov r0, #1\n"
		"mov r1, #0x11\n"
		"svc 2\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			/* no inputs */
		:
			"r0", "memory"
	);

	return ret;
}

int beep(int freq, int dur) {
	int ret;

	asm volatile(
		"mov r0, #2\n"
		"mov r1, %[freq]\n"
		"mov r2, %[dur]\n"
		"svc 2\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			[freq] "l" (freq),
			[dur] "l" (dur)
		:
			"r0", "r1", "r2", "memory"
	);

	return ret;
}

int window(int x1, int y1, int x2, int y2) {
	int area_data[4] = {x1, y1, x2, y2};
	int ret;

	asm volatile(
		"mov r0, #1\n"
		"mov r1, #18\n"
		"mov r2, %[data]\n"
		"svc 2\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			[data] "l" (area_data)
		:
			"r0", "r1", "r2", "memory"
	);

	return ret;
}

int set_display_coordinate_mode(int mode) {
	int ret;

	asm volatile(
		"mov r0, #1\n"
		"mov r1, #34\n"
		"mov r2, %[mode]\n"
		"svc 2\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			[mode] "l" (mode)
		:
			"r0", "r1", "r2", "memory"
	);

	return ret;
}

int set_display_color(int type, int color) {
	int ret;

	asm volatile(
		"mov r0, #1\n"
		"mov r1, #33\n"
		"mov r2, %[type]\n"
		"mov r3, %[color]\n"
		"svc 2\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			[type] "l" (type),
			[color] "l" (color)
		:
			"r0", "r1", "r2", "r3", "memory"
	);

	return ret;
}

int putpixelcol(char *buf, int len) {
	int ret;

	asm volatile(
		"mov r0, #1\n"
		"mov r1, #26\n"
		"mov r2, %[buf]\n"
		"mov r3, %[len]\n"
		"svc 2\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			[buf] "l" (buf),
			[len] "l" (len)
		:
			"r0", "r1", "r2", "r3", "memory"
	);

	return ret;
}

// Not verified to work yet
int put_graphic(int x1, int y1, int x2, int y2, int len, void *buf) {
	int ret;
	int graphic_data[6] = {x1, y1, x2, y2, len, (int)buf};

	asm volatile(
		"mov r0, #1\n"
		"mov r1, #27\n"
		"mov r2, %[graphic_data]\n"
		"svc 2\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			[graphic_data] "l" (graphic_data)
		:
			"r0", "r1", "r2", "memory"
	);

	return ret;
}

int write_pixels (int x1, int y1, int x2, int y2, int color) {
	  int ret;
	  int pixel_data[5] = {x1, y1, x2, y2, color};

	  asm volatile(
		"mov r0, #1\n"
		"mov r1, #35\n"
		"mov r2, %[pixel_data]\n"
		"svc 2\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			[pixel_data] "l" (pixel_data)
		:
			"r0", "r1", "r2", "memory"
	  );

	  return ret;
}


int display_frame_buffer (int x, int y, int w, int h, short *buf) {
	  int ret;
	  int buffer_data[5] = {x, y, w, h, (int)buf};

	  asm volatile(
		"mov r0, #1\n"
		"mov r1, #42\n"
		"mov r2, %[buffer_data]\n"
		"svc 2\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			[buffer_data] "l" (buffer_data)
		:
			"r0", "r1", "r2", "memory"
	  );

	  return ret;
}

int alt_display_frame_buffer(int fd, int x, int y, int w, int h, short *buf) {
	  int ret;
	  int buffer_data[5] = {x, y, w, h, (int)buf};

	  asm volatile(
		"mov r0, %[fd]\n"
		"mov r1, #42\n"
		"mov r2, %[buffer_data]\n"
		"svc 2\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			[fd] "l" (fd),
			[buffer_data] "l" (buffer_data)
		:
			"r0", "r1", "r2", "memory"
	  );

	  return ret;
}

int put_bmp(int fd, char *path) {
	int ret;

  asm volatile(
	"mov r0, %[fd]\n"
	"mov r1, #37\n"
	"mov r2, %[path]\n"
	"svc 2\n"
	"mov %[ret], r0\n"
	:
		[ret] "=l" (ret)
	:
		[fd] "l" (fd),
		[path] "l" (path)
	:
		"r0", "r1", "r2", "memory"
  );

	return ret;
}

int get_touchscreen(int *x, int *y) {
	int ret;
	int touch_data[3];

	touch_data[0] = 1;

  asm volatile(
	"mov r0, #1\n"
	"mov r1, #13\n"
	"mov r2, %[touch_data]\n"
	"svc 3\n"
	"mov %[ret], r0\n"
	:
		[ret] "=l" (ret)
	:
		[touch_data] "l" (touch_data)
	:
		"r0", "r1", "r2", "memory"
  );

  	if (ret) {
	  	*x = touch_data[1];
	  	*y = touch_data[2];
  	}

	return ret;
}


int wait(unsigned msec) {
	int ret;
	
	asm volatile(
		"mov r0, #2\n"
		"mov r1, %[msec]\n"
		"svc 10\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			[msec] "l" (msec)
		:
			"r0", "r1", "memory"
	);

	return ret;
}

int read_ticks() {
	int ret;
	
	asm volatile(
		"mov r0, #9\n"
		"svc 10\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
		:
			"r0", "memory"
	);

	return ret;
}

int set_opn_blk(int handle, const opn_blk_t *opn_blk) {
	int ret;

	asm volatile(
		"mov r0, %[handle]\n"
		"mov r1, #0\n"
		"mov r2, %[opn_blk]\n"
		"svc 2\n"
		"mov %[ret], r0\n"
		:
			[ret] "=l" (ret)
		:
			[handle] "l" (handle),
			[opn_blk] "l" (opn_blk)
		:
			"r0", "r1", "r2", "memory"
	);

	return ret;

	//{ return _control(hdl, 0, (char*)ob); }
}





/* Non-syscall library functions */

unsigned strlen(const char *str) {
	int ret = 0;
	while (*str) {
		ret++;
		str++;
	}
	return ret;
}

void strncpy(char *dst, const char *src, unsigned n) {
	for (unsigned i = 0; i < n; ++i) {
		dst[i] = src[i];
	}
}

int fputc(char c, int fd) {
	write(fd, &c, 1);
}

int fputs(char *s, int fd) {
	write(fd, s, strlen(s));
}

// Adapted from:
// https://stackoverflow.com/questions/3440726/what-is-the-proper-way-of-implementing-a-good-itoa-function
int itoa_hex(unsigned v, char *sp) {
	char tmp[16];
	char *tp = tmp;
	int i;

	while (v || tp == tmp)
	{
		i = v % 16;
		v /= 16;
		if (i < 10)
		  *tp++ = i+'0';
		else
		  *tp++ = i + 'a' - 10;
	}

	int len = tp - tmp;

	if (len == 1) 
	{
		*sp++ = '0';
		len++;
	}

	while (tp > tmp)
		*sp++ = *--tp;

	*sp++ = '\x00';

	return len;
}


void hexdump(char *addr, size_t n, int stream) {
	size_t i = 0;
	char digitbuf[5];
	fputs("HEXDUMP: ", stream);
	for (char *p = addr; p < addr + n; ++p) {
		itoa_hex(*p, digitbuf);
		fputs(digitbuf, stream);
		fputs(" ", stream);
		if ((int)p % 8 == 0) {
			fputs("\r\n", stream);
		}
	}
}


// // Simplified, from:
// // https://stackoverflow.com/questions/16647278/minimal-implementation-of-sprintf-or-printf
// int fprintf(int fd, char const *fmt, void **args) {

// 	int int_temp;
// 	char char_temp;
// 	char *string_temp;

// 	int args_idx = 0;

// 	char ch;
// 	int length = 0;

// 	char buffer[512];

// 	while ( ch = *fmt++) {
// 		if ( '%' == ch ) {
// 			switch (ch = *fmt++) {
// 				/* %% - print out a single %    */
// 				case '%':
// 					fputc('%', fd);
// 					length++;
// 					break;

// 				/* %c: print out a character    */
// 				case 'c':
// 					char_temp = (int)args[args_idx++];
// 					fputc(char_temp, fd);
// 					length++;
// 					break;

// 				/* %s: print out a string       */
// 				case 's':
// 					string_temp = (char *)args[args_idx++];
// 					fputs(string_temp, fd);
// 					length += strlen(string_temp);
// 					break;

// 				/* %x: print out an int in hex  */
// 				case 'd':
// 				case 'x':
// 					int_temp = (int)args[args_idx++];
// 					itoa_hex(int_temp, buffer);
// 					fputs(buffer, fd);
// 					length += strlen(buffer);
// 					break;
// 			}
// 		}
// 		else {
// 			fputc(ch, fd);
// 			length++;
// 		}
// 	}
// 	return length;
// }
