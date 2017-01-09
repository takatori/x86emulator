#include "emulator_function.h"

void push32(Emulator* emu, uint32_t value) {
  
  uint32_t address = get_register32(emu, ESP) - 4;
  /* 先にespを減らしてから */
  set_register32(emu, ESP, address);
  /* スタックトップに値を書き込む */
  set_memory32(emu, address, value);
}

uint32_t pop32(Emulator* emu) {

  uint32_t address = get_register32(emu, ESP);
  /* 先に値を読み出してから */
  uint32_t ret     = get_memory32(emu, address);
  /* espを増やす */
  set_register32(emu, ESP, address + 4);
  return ret;
}

/* memory配列の指定した番地から8ビットの値を取得する関数 
   関数の第二引数にその時のeipからオフセットを指定するとその番地から値を読み取って返す*/
uint32_t get_code8(Emulator* emu, int index) {
  return emu->memory[emu->eip + index];
}

/* memory配列の指定した番地から8ビットのint値を取得する関数 */
int32_t get_sign_code8(Emulator* emu, int index) {
  return (int8_t)emu->memory[emu->eip + index];
}

/* memory配列の指定した番地から32ビットの値を取得する関数 */
uint32_t get_code32(Emulator* emu, int index) {
  int i;
  uint32_t ret = 0;

  /* i386はリトルエンディアンを採用しているので、
     リトルエンディアンでメモリの値を取得する */
  for(i = 0; i < 4; i++) {
    /* for文で1バイトを読み取るごとに8ビットずつ左にずらす処理を行う */
    /* `|=` ビット単位のOR代入演算子 */
    ret |= get_code8(emu, index + i) << (i * 8);
  }

  return ret;
}

int32_t get_sign_code32(Emulator* emu, int index) {
  return (int32_t)get_code32(emu, index);
}

uint32_t get_register32(Emulator* emu, int index) {
  return emu->registers[index];
}

void set_register32(Emulator* emu, int index, uint32_t value) {
  emu->registers[index] = value;
}

uint32_t get_memory8(Emulator* emu, uint32_t address) {
  return emu->memory[address];
}

uint32_t get_memory32(Emulator* emu, uint32_t address) {
  int i;
  uint32_t ret = 0;
  /* リトルエンディアンで書かれた32ビット値をuint32_t型に変換するために、
     for文で１バイトずつ読み取って左にずらし、 
     前回のretの値にビットORをしている*/
  for(i = 0; i < 4; i++) {
    ret |= get_memory8(emu, address + i) << (8 * i);
  }
  return ret;
}

void set_memory8(Emulator* emu, uint32_t address, uint32_t value) {
  emu->memory[address] = value & 0xFF;
}

/* 32ビット値をリトルエンディアンでメモリに書き込む */
void set_memory32(Emulator* emu, uint32_t address, uint32_t value) {
  int i;
  for(i = 0; i < 4; i++) {
    set_memory8(emu, address + i, value >> (i * 8));
  }
}
