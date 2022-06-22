TOOLCHAIN_PREFIX = aarch64-linux-gnu-
CC = $(TOOLCHAIN_PREFIX)gcc
LD = $(TOOLCHAIN_PREFIX)ld
OBJCPY = $(TOOLCHAIN_PREFIX)objcopy

BUILD_DIR = build
SRC_DIR = src
LIB_DIR = lib

LINKER_FILE = $(SRC_DIR)/linker.ld
SRCS_C = $(wildcard $(SRC_DIR)/*.c)
OBJS_C = $(SRCS_C:$(SRC_DIR)/%.c=$(BUILD_DIR)/c/%.o)
SRCS_ASM = $(wildcard $(SRC_DIR)/*.S)
OBJS_ASM = $(SRCS_ASM:$(SRC_DIR)/%.S=$(BUILD_DIR)/asm/%.o)
SRCS_LIB = $(wildcard $(LIB_DIR)/*.c)
OBJS_LIB = $(SRCS_LIB:$(LIB_DIR)/%.c=$(BUILD_DIR)/lib/%.o)
 
CFLAGS = -Wall -I include -c -ffreestanding -nostdinc -nostdlib -nostartfiles -g

.PHONY: all clean

all: build_dir kernel8.img

# build

$(BUILD_DIR)/c/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/lib/%.o: $(LIB_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/asm/%.o: $(SRC_DIR)/%.S
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@

kernel8.img: $(OBJS_C) $(OBJS_ASM) $(OBJS_LIB)
	$(LD) $(OBJS_C) $(OBJS_ASM) $(OBJS_LIB) -T $(LINKER_FILE) -o kernel8.elf
	$(OBJCPY) -O binary kernel8.elf kernel8.img 

asm:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -display none -d in_asm

run: $(BUILD_DIR) kernel8.img
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -dtb bcm2710-rpi-3-b-plus.dtb -initrd initramfs.cpio -serial null -serial stdio -display none -drive if=sd,file=sfn_nctuos.img,format=raw

debug: $(BUILD_DIR) kernel8.img
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -dtb bcm2710-rpi-3-b-plus.dtb -initrd initramfs.cpio -display none -S -s -serial null -serial stdio -drive if=sd,file=sfn_nctuos.img,format=raw

send: $(BUILD_DIR) kernel8.img sendimg.py
	sudo chmod 777 /dev/ttyUSB0
	python3 sendimg.py kernel8.img /dev/ttyUSB0
	sudo screen /dev/ttyUSB0 115200

cpio:
	cd rootfs
	find . | cpio -o -H newc > ../initramfs.cpio

build_dir: $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)/*
	rm -f *.elf *.img

test:
	cd src
	echo 'test' > test.txt
