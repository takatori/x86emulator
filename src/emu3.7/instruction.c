#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "instruction.h"
#include "emulator.h"
#include "emulator_function.h"
#include "modrm.h"

instruction_func_t* instructions[256];

/* オペコードの下位3ビットにレジスタ番号が埋め込まれているタイプの命令 */
/* push r32は50+rdなので、ベース地にレジスタ番号を足したものがオペコードになっていることがわかる */
static void push_r32(Emulator* emu) {
  /* オペコードからベース値を引き算することでレジスタ番号を得ることができる */
  uint8_t reg = get_code8(emu, 0) - 0x50;
  /* 得られたレジスタ番号を元にレジスタから値を読み込み、
     emu->espが指すスタックトップにプッシュ */
  push32(emu, get_register32(emu, reg));
  emu->eip += 1;
}

static void push_imm32(Emulator* emu) {
  uint32_t value = get_code32(emu, 1);
  push32(emu, value);
  emu->eip += 5;
}

static void push_imm8(Emulator* emu) {
  uint8_t value = get_code8(emu, 1);
  push32(emu, value);
  emu->eip += 2;
}

/* オペコードの下位3ビットにレジスタ番号が埋め込まれているタイプの命令 */
/* pop r32は58+rdなので、ベース地にレジスタ番号を足したものがオペコードになっていることがわかる */
static void pop_r32(Emulator* emu) {
  uint8_t reg = get_code8(emu, 0) - 0x58;
  set_register32(emu, reg, pop32(emu));
  emu->eip += 1;
}

/* callの次の命令の番地を基準にして前後32ビットの範囲でジャンプできる命令 */
static void call_rel32(Emulator* emu) {
  /* 1バイトのオペコードの次に32ビットの符号付き整数が来ることになっているので、
     get_sign_code32で読み取る */
  int32_t diff = get_sign_code32(emu, 1);
  /* callの直後に来る命令の番地を計算しスタックにpushする */
  /* このcall命令は全体で5バイトであるため、直後に来る命令はemu->eip + 5に配置される */
  /* 直後の命令の先頭番地をpushしておくことで、あとでretに戻るときにきちんと処理が継続する */
  push32(emu, emu->eip + 5);
  /* 目的地にジャンプするためにeipを書き換える */
  emu->eip += (diff + 5);
}

/* mov esp, ebpとpop ebpをまとめて実行する命令　 */
static void leave(Emulator* emu) {
  uint32_t ebp = get_register32(emu, EBP);
  set_register32(emu, ESP, ebp);
  set_register32(emu, EBP, pop32(emu));
  emu->eip += 1;
}

static void ret(Emulator* emu) {
  emu->eip = pop32(emu);
}

static void add_rm32_imm8(Emulator* emu, ModRM* modrm) {
  uint32_t rm32 = get_rm32(emu, modrm);
  uint32_t imm8 = (int32_t)get_sign_code8(emu, 0);
  emu->eip += 1;
  set_rm32(emu, modrm, rm32 + imm8);
}

/* 加算をおこなう */
static void add_rm32_r32(Emulator* emu) {
  
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  uint32_t r32  = get_r32(emu, &modrm);
  uint32_t rm32 = get_rm32(emu, &modrm);
  set_rm32(emu, &modrm, rm32 + r32);
}

static void sub_rm32_imm8(Emulator* emu, ModRM* modrm) {

  uint32_t rm32 = get_rm32(emu, modrm);
  uint32_t imm8 = (int32_t)get_sign_code8(emu, 0);
  emu->eip += 1;
  set_rm32(emu, modrm, rm32 - imm8);
}

/* 減算を行う */
/* この減算命令はModR/MのREGビットをオペコードの拡張として使うタイプの命令
   最初の１バイトだけでは実際の命令が決まらない*/
static void code_83(Emulator* emu) {

  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);

  switch(modrm.opecode) {
  case 0:
    add_rm32_imm8(emu, &modrm);
    break;
    /* REGビットが5のときにsub_rm32_imm8を呼び指す */
  case 5:
    sub_rm32_imm8(emu, &modrm);
    break;
  default:
    printf("not implemented: 83 /%d\n", modrm.opecode);
    exit(1);
  }
}

static void inc_rm32(Emulator* emu, ModRM* modrm) {
  uint32_t value = get_rm32(emu, modrm);
  set_rm32(emu, modrm, value + 1);
}

/* インクリメントを行う */
/* ModR/MのREGビットが0のときにincだと決まる。 */
static void code_off(Emulator* emu) {
  
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);

  switch(modrm.opecode) {
  case 0:
    inc_rm32(emu, &modrm);
    break;
  default:
    printf("not imiplemented: FF /%d\n", modrm.opecode);
    exit(1);
  }
}

/* 汎用レジスタに32ビットの即値をコピーするmov命令に対応する */
static void mov_r32_imm32(Emulator* emu) {
  /* このmov命令のオペコードはrをレジスタ番号だとすると0xb8+r */
  /* オペコード自身がレジスタの指定を含むタイプの命令 */
  uint8_t  reg   = get_code8(emu, 0) - 0xB8;
  /* オペコードのすぐ後に32ビットの即値がくるはずなので、
     get_code32で32ビット値を読み取ってレジスタに代入している */
  uint32_t value = get_code32(emu, 1);

  emu->registers[reg] = value;
  emu->eip += 5;  
}

/* 32ビットの即値を、ModR/Mで指定されたRegisterまたは
   メモリ領域に書き込む機械語(オペコード0xc7) */
static void mov_rm32_imm32(Emulator* emu) {
  /* 関数が呼ばれたときemu->eipはオペコードを指した状態なので、
     インクリメントしてModR/Mバイトを指すように調整 */
  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  /* parse_modrmから戻るとemu->eipは即値を指しているはずなので、32ビットの即値を読み取る */
  uint32_t value = get_code32(emu, 0);
  emu->eip += 4;
  /* modrmの設定に従って値を書き込む */
  set_rm32(emu, &modrm, value);
}

/* rm32から32ビットを読み取りr32に書き込む */
static void mov_r32_rm32(Emulator* emu) {

  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  uint32_t rm32 = get_rm32(emu, &modrm);
  set_r32(emu, &modrm, rm32);
}

/* r32から32ビットを読み取りrm32に書き込む */
static void mov_rm32_r32(Emulator* emu) {

  emu->eip += 1;
  ModRM modrm;
  parse_modrm(emu, &modrm);
  uint32_t r32 = get_r32(emu, &modrm);
  set_rm32(emu, &modrm, r32);
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
  instructions[0x01] = add_rm32_r32;

  for(i = 0; i < 8; i++) {
    instructions[0x50 + i] = push_r32;
  }

  for(i = 0; i < 8; i++) {
    instructions[0x58 + i] = pop_r32;
  }

  instructions[0x68] = push_imm32;
  instructions[0x6A] = push_imm8;
  
  instructions[0x83] = code_83;
  instructions[0x89] = mov_rm32_r32;
  instructions[0x8B] = mov_r32_rm32;
  
  for(i = 0; i < 8; i++) {
    instructions[0xB8 + i] = mov_r32_imm32;
  }

  instructions[0xC3] = ret;
  instructions[0xC7] = mov_rm32_imm32;
  instructions[0xC9] = leave;

  instructions[0xE8] = call_rel32;
  instructions[0xE9] = near_jump;
  instructions[0xEB] = short_jump;
  instructions[0xFF] = code_off;
}
