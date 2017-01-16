CFLAGS += -Wall -c
OBJS = main.o emulator_function.o instruction.o modrm.o io.o
CC = gcc

all :
	make x86

x86: $(OBJS) Makefile
	$(CC) -o x86 $(OBJS)

%.o: %c. Makefile
	$(CC) $(CFLAGS) -o $@ $<
