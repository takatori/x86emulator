TARGET = in.bin out.bin select.bin

CC = gcc
LD = ~/.local/binutils/bin/i386-unknown-linux-gnu-ld
AS = nasm
CFLAGS += -nostdlib -fno-asynchronous-unwind-tables \
	 -g -fno-stack-protector -Wall
LDFLAGS += --entry=func -oformat=binary

.PHONY: all
all :
	make $(TARGET)

%.o : %.c Makefile
	$(CC) $(CFLAGS) -c $<

%.bin : %.o Makefile
	$(LD) $(LDFLAGS) -o $@ $<

%.bin : %.asm Makefile
	$(AS) -f bin $<
