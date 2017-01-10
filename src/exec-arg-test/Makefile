TARGET = test.bin
OBJS = crt0.o test.o

CC = gcc
LD = ~/.local/binutils/bin/i386-unknown-linux-gnu-ld
AS = nasm
CFLAGS += -nostdlib -fno-asynchronous-unwind-tables \
	 -g -fno-stack-protector -target i386-elf
LDFLAGS += --entry=start -m elf_i386 --oformat=binary -Ttext 0x7c00 

.PHONY: all
all :
	make $(TARGET)

%.o : %.c Makefile
	$(CC) $(CFLAGS) -c $<

%.o : %.asm Makefile
	$(AS) -f elf $<

$(TARGET) : $(OBJS) Makefile
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
