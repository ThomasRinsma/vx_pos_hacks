
#include "lib.h"

#define WIDTH 240
#define HEIGHT 320

void exit_wait();

void main() {
	char buf[10];
	int ret = 99;
	int evt;
	
	int console = open("/DEV/CONSOLE", 2);
	clear_screen();

	int tty = open("/DEV/COM1", 2);
	if (tty == -1) {
		fputs("error opening TTY\n\n", console);
		exit_wait();
	}

	static const opn_blk_t tty_info = {
		.rate = 9,   // 115200 baud
		.format = 4, // A8N1
		.protocol = 0, // character based
		// .param;
	};
	set_opn_blk(tty, &tty_info);

	// fputs("Hax!", console);
	// wait(500);

	// hexdump((char *)0x70000040, 1024, tty);

	// while(1) {}
	// fputs("Waiting 15 seconds...", console);
	// wait(15000);

	// char *test_str = (char *)0x70424000;
	// sprintf(test_str, "Testing: '%d'\n", 42);
	// fputs(test_str, console);

	int frame = open("/DEV/FRAME", 0);
	if (frame == -1) {
		fputs("error opening /DEV/FRAME\n\n", console);
		exit_wait();
	}

	set_display_coordinate_mode(1); // pixel drawing mode

	// Arrays
	short *pixels = (short *)0x70424000;
	char *cells = (char *)pixels + WIDTH*HEIGHT*2;
	char *neighbours = cells + WIDTH*HEIGHT;
	// short *pixels = (short *)malloc(WIDTH * HEIGHT * 2);
	// char *cells = malloc(WIDTH * HEIGHT);
	// char *neighbours = malloc(WIDTH * HEIGHT);

	// Initialize
	for (int i = 0; i < WIDTH*HEIGHT; ++i) {
		cells[i] = 0;
	}


	int playing = 0;
	
	// set up field with touch
	while (1) {
		// set touched point
		int x, y, touched;
		touched = get_touchscreen(&x, &y);
		if (touched) {
			cells[y*WIDTH+x] = 1;
			cells[(y-2)*WIDTH+(x-2)] = 1;
			cells[(y-2)*WIDTH+(x+2)] = 1;
			cells[(y+2)*WIDTH+(x-2)] = 1;
			cells[(y+2)*WIDTH+(x+2)] = 1;
		}

		// pixels from cells
		for (int x = 0; x < WIDTH; ++x) {
			for (int y = 0; y < HEIGHT; ++y) {
				pixels[y*WIDTH+x] = cells[y*WIDTH+x] ? 0xFFFF : 0x0000;
			}
		}

		if (playing) {

			for (int i = 0; i < WIDTH; ++i) {
				// horizontal edges
				neighbours[0*WIDTH+i] = 0;
				neighbours[(HEIGHT-1)*WIDTH+i] = 0;
			}
			for (int i = 0; i < HEIGHT; ++i) {
				// vertical edges
				neighbours[i*WIDTH+0] = 0;
				neighbours[i*WIDTH+(HEIGHT-1)] = 0;
			}
			// main area
			for (int x = 1; x < WIDTH-1; ++x) {
				for (int y = 1; y < HEIGHT-1; ++y) {
					neighbours[y*WIDTH+x] = 
						cells[(y-1)*WIDTH+(x-1)] + 
						cells[(y-1)*WIDTH+x] + 
						cells[(y-1)*WIDTH+(x+1)] + 
						cells[y*WIDTH+(x-1)] + 
						cells[y*WIDTH+(x+1)] + 
						cells[(y+1)*WIDTH+(x-1)] + 
						cells[(y+1)*WIDTH+x] + 
						cells[(y+1)*WIDTH+(x+1)];
				}
			}


			// compute new cells
			char *cell;
			int n;
			for (int x = 0; x < WIDTH; ++x) {
				for (int y = 0; y < HEIGHT; ++y) {
					cell = &cells[y*WIDTH+x];
					n = neighbours[y*WIDTH+x];

					if (*cell && (n < 2 || n > 3)) {
						*cell = 0;
					} else if (!(*cell) && n == 3) {
						*cell = 1;
					}
				}
			}

			// pixels from cells
			for (int x = 0; x < WIDTH; ++x) {
				for (int y = 0; y < HEIGHT; ++y) {
					pixels[y*WIDTH+x] = cells[y*WIDTH+x] ? 0xFFFF : 0x0000;
				}
			}
		}

		alt_display_frame_buffer(frame, 0, 0, WIDTH, HEIGHT, pixels);

		evt = wait_event();
		if (evt & 1) {
			// switch state
			playing = !playing;
		}
	}

	exit_wait();
}

void exit_wait() {
	while (1) {
		wait_event();
	}
}

