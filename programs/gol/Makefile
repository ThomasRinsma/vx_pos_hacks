## === Customize this for your device ===
OS_VERSION = QT000500
SBI_VERSION = 03_10
TTY = /dev/ttyUSB0
# =======================================

TOOLCHAIN_DIR = ../../toolchain
OUT_FILE = H.OUT
TRIPLET = arm-none-eabi
CC = $(TRIPLET)-gcc
LD = $(TRIPLET)-ld
OBJDUMP = $(TRIPLET)-objdump
OBJCOPY = $(TRIPLET)-objcopy
CFLAGS = -Werror -O1 -mbig-endian -fPIE -nostdlib -mthumb -mthumb-interwork -ffreestanding -march=armv4t -fomit-frame-pointer

OBJECTS = main.o lib.o

.PHONY: upload clean all

all: H.OUT

$(OUT_FILE): $(OBJECTS)
	# link
	$(LD) $(OBJECTS) -T $(TOOLCHAIN_DIR)/script.ld -o $@_elf

	# copy just .text, .rodata and .bss
	$(OBJCOPY) -O binary $@_elf $@_binary

	# add header and create OUT file
	cat $(TOOLCHAIN_DIR)/base.out $@_binary > $@

	# fix header fields
	python $(TOOLCHAIN_DIR)/fix_size.py $(OUT_FILE)

	# dissasembly
	$(OBJDUMP) -d $@_elf

	# delete temporaries
	rm $@_elf $@_binary

$(OBJECTS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@


upload:
	@python $(TOOLCHAIN_DIR)/send_file_and_auth.py -d $(TTY) -o $(OS_VERSION) -s $(SBI_VERSION) -f $(OUT_FILE)
	
clean:
	rm *.o H.OUT