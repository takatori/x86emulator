#include "io.h"

#include <stdio.h>
#include "emulator.h"

uint8_t io_in8(uint16_t address) {
  switch (address) {
  case 0x03f8:
    return getchar();
  default:
    return 0;
  }
}

void io_out8(uint16_t address, uint8_t value) {
  switch(address) {
  case 0x03f8:
    putchar(value);
    break;
  }
}
