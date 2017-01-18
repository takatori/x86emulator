TARGET = x86
OBJS = main.o emulator_function.o instruction.o modrm.o io.o bios.o

CC = gcc
CFLAGS += -Wall

.PHONY: all
all :
	make $(TARGET)

%.o : %.c Makefile
	$(CC) $(CFLAGS) -c $<

$(TARGET) : $(OBJS) Makefile
	$(CC) -o $@ $(OBJS)
