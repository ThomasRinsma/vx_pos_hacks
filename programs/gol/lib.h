typedef unsigned long size_t;


// typedef int (*sprintf_ptr)(char *, const char *, ...);
// extern sprintf_ptr sprintf;

int sprintf(char *, const char *, ...);

// typedef char *(*malloc_ptr)(size_t);
// extern malloc_ptr malloc;

char *malloc(size_t size);


// ============= SYSCALLS ===============

typedef struct opn_blk_s {
	char rate;
	char format;
	char protocol;
	char param;
	char unknown[8];
} opn_blk_t;


// File I/O
int open(const char *, int flags);
int close(int fd);
int write(int, const char *, unsigned);
int read(int fd, char *buf, unsigned len);
int set_opn_blk(int handle, const opn_blk_t *opn_blk);

// other
int enable_hot_key();
int wait_event();
int beep(int freq, int dur);
int wait(unsigned msec);
int read_ticks();

// screen-specific
int clear_screen();
int window(int x1, int y1, int x2, int y2);
int set_display_coordinate_mode(int mode);
int set_display_color(int type, int color);
int putpixelcol(char *buf, int len);
int put_graphic(int x1, int y1, int x2, int y2, int len, void *buf);
int display_frame_buffer (int x, int y, int w, int h, short *buf);
int alt_display_frame_buffer(int fd, int x, int y, int w, int h, short *buf);
int write_pixels (int x1, int y1, int x2, int y2, int color);
int put_bmp(int fd, char *path);
int get_touchscreen(int *x, int *y);

// ========================================

// Non-syscall useful library funcs
unsigned strlen(const char *str);
void strncpy(char *dst, const char *src, unsigned n);
int fputc(char c, int fd);
int fputs(char *s, int fd);
int itoa_hex(unsigned value, char *sp);
// int fprintf(int fd, char const *fmt, void **args);
void hexdump(char *addr, size_t n, int stream);