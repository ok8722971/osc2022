.PHONY: all clean random

all: ssd_fuse ssd_fuse_dut

ssd_fuse: ssd_fuse.c
	gcc -Wall ssd_fuse.c `pkg-config fuse3 --cflags --libs` -D_FILE_OFFSET_BITS=64 -o ssd_fuse

ssd_fuse_dut: ssd_fuse_dut.c
	gcc -Wall ssd_fuse.c `pkg-config fuse3 --cflags --libs` -D_FILE_OFFSET_BITS=64 -o ssd_fuse
	gcc -Wall ssd_fuse_dut.c -o ssd_fuse_dut

run: all
	mkdir /tmp/ssd
	./ssd_fuse -d /tmp/ssd

clean:
	rm -rf /tmp/ssd
	rm ssd_fuse ssd_fuse_dut
	rm nand_*
	clear

