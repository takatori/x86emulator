TARGET = test.bin
OBJS = crt0.o test.o

CC = gcc
LD = ld
AS = nasm
CFLAGS += -nostdlib -fno-asynchronous-unwind-tables \
	 -g -fno-stack-protector -Wall
LDFLAGS += -e start
# --oformat=binary -Ttext 0x7c00 
# LDFLAGS +=  -macosx_version_min 10.10 -lSystem

.PHONY: all
all :
	make $(TARGET)

%.o : %.c Makefile
	$(CC) $(CFLAGS) -c $<

%.o : %.asm Makefile
	$(AS) -f elf $<

$(TARGET) : $(OBJS) Makefile
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
