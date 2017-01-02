#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "emulator.h"
#include "emulator_function.h"
#include "instruction.h"

char* registers_name[] = {
  "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"};

/* メモリは1MB */
#define MEMORY_SIZE (1024 * 1024)


/* 汎用時レスタとプログラムカウンタの値を標準出力に出力する */
static void dump_registers(Emulator* emu) {
  
  int i;

  for (i = 0; i < REGISTERS_COUNT; i++) {
    printf("%s = %08x\n", registers_name[i], emu->registers[i]);
  }

  printf("EIP = %08x\n", emu->eip);
  
}

/* エミュレータを作成する */
Emulator* create_emu(size_t size, uint32_t eip, uint32_t esp) {

  Emulator* emu = malloc(sizeof(Emulator));
  emu->memory   = malloc(size);

  /* 汎用レジスタの初期値をすべて0にする */
  memset(emu->registers, 0, sizeof(emu->registers));

  /* レジスタの初期値を指定されたものにする */
  emu->eip            = eip;
  emu->registers[ESP] = esp;

  return emu;
  
}

/* エミュレータを破棄する */
void destroy_emu(Emulator* emu) {
  free(emu->memory);
  free(emu);
}


/* 
エミュレータ構造体を生成して初期化し、ファイルから機械語プログラムを読み込む処理 
コマンドライン引数に機械語プログラムが格納されたファイルを指定する
*/
int main(int args, char* argv[]) {

  FILE* binary;
  Emulator* emu;

  /* コマンドライン引数が一つ指定されていることを確認 */
  if(args != 2) {
    printf("usage: x86 filename\n");
    return 1;
  }

  /* EIPが0、ESPが0x7C00の状態のエミュレータを作る */
  /* 左からメモリ容量、eipの初期値、espの初期値 */
  emu = create_emu(MEMORY_SIZE, 0x7c00, 0x7c00);

  binary = fopen(argv[1], "rb");
  if(binary == NULL) {
    printf("%sファイルが開けません\n", argv[1]);
    return 1;
  }

  /* 機械語ファイルを読み込む(最大512バイト) */
  /* memoryの先頭ではなく0x7c00番地から機械語を配置する */
  /* この番地とeipの初期値、orgの値の３つの値が一致していて初めて正しくプログラムが動作する */
  fread(emu->memory + 0x7c00, 1, 0x200, binary);
  fclose(binary);


  init_instructions();

  while(emu->eip < MEMORY_SIZE) {
    uint8_t code = get_code8(emu, 0);
    if(instructions[code] == NULL) {
      printf("\n\nNot Implemented: %x\n", code);
      break;
    }
    
    /* 命令の実行 */
    instructions[code](emu);
    
    /* 一つの命令を実行するたびにeipをチェックし、0ならメインループを終了する */
    /* 普通のCPUには終了機能はないが、エミュレータではプログラムの修了時にレジスタの値を表示したいので、明示的に終了させる仕組みが必要 */
    if(emu->eip == 0x00) {
      printf("\n\nend of program. \n\n");
      break;
    }
  }

  dump_registers(emu);
  destroy_emu(emu);
  return 0;  
}
