#include <stdint.h>
#include <string.h>

#include "instruction.h"
#include "emulator.h"
#include "emulator_function.h"

instruction_func_t* instructions[256];

/* 汎用レジスタに32ビットの即値をコピーするmov命令に対応する */
void mov_r32_imm32(Emulator* emu) {
  /* このmov命令のオペコードはrをレジスタ番号だとすると0xb8+r */
  /* オペコード自身がレジスタの指定を含むタイプの命令 */
  uint8_t  reg   = get_code8(emu, 0) - 0xB8;
  /* オペコードのすぐ後に32ビットの即値がくるはずなので、
     get_code32で32ビット値を読み取ってレジスタに代入している */
  uint32_t value = get_code32(emu, 1);

  emu->registers[reg] = value;
  emu->eip += 5;  
}

/* 1バイトのメモリ番地を取るjump命令、ショートジャンプ命令に対応する */
/* この命令はオペランドにバイトの符号付き整数(つまり2の補数表現で解釈される)を取りeipに加算する
   したがって現在地から前に127バイト、後ろに128バイトの範囲内でジャンプすることができる */
void short_jump(Emulator* emu) {
  /* オペランドを8ビット符号付き整数としてdiffに読み込む */
  int8_t diff = get_sign_code8(emu, 1);
  /* jump命令はその次の命令の番地を起点にjump先を計算するので、
     eipにはdiff + 2(ショートジャンプ命令は2バイト命令)を加算する*/
  emu->eip += (diff + 2);
}

/* 32ビットの符号つき整数を取る相対ジャンプ命令 */
void near_jump(Emulator* emu) {
  int32_t diff = get_sign_code32(emu, 1);
  emu->eip += (diff + 5);
}


void init_instructions(void) {
  int i;
  memset(instructions, 0, sizeof(instructions));
  for(i = 0; i < 8; i++) {
    instructions[0xB8 + i] = mov_r32_imm32;
  }
  instructions[0xE9] = near_jump;
  instructions[0xEB] = short_jump;  
}
