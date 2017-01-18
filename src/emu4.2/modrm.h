#ifndef MODRM_H_
#define MODRM_H_

#include <stdint.h>

#include "emulator.h"

/* ModR/Mを表す構造体 */
typedef struct {
  
  uint8_t mod;

  /* opecodeとreg_indexは別名で同じ物 */
  /* 共用体になっているため、opecodeに値を代入すると同時に
     reg_indexにも同じ値が入るようになっている */
  union {
    uint8_t opecode;
    uint8_t reg_index;
  };
  
  uint8_t rm;

  /* SIBが必要な mod/rm の組み合わせのときに使う */
  uint8_t sib;
  
  union {
    int8_t disp8; //disp8は符号付き整数
    uint32_t disp32;    
  };
} ModRM;

/* ModR/M, SIB, ディプレースメントを解析する
 * 
 * emuからModR/M, SIB, ディプレースメントを読み取ってmodrmにセットする。
 * 呼び出しのときemu->eipはModR/Mバイトを指している必要がある。
 * この関数はemu->eipを即値(即値がない場合は次の命令)の先頭を指すように変更する。
 *
 * 引数
 *  emu: eipがModR/Mバイトの先頭を指しているエミュレータ構造体
 *  modrm: 解析結果を格納するための構造体
 */
void parse_modrm(Emulator* emu, ModRM* modrm);


/* ModR/Mの内容に基づきメモリの実行アドレスを計算する
 * 
 * modrm->modは0,1,2のいずれかでなければならない
 */
uint32_t calc_memory_address(Emulator* emu, ModRM* modrm);

/* rm32のレジスタまたはメモリの32bit値を取得する */
uint32_t get_rm32(Emulator* emu, ModRM* modrm);

/* rm32のRegisterまたはメモリの32bit値を設定する
 *
 * modrmの内容に従ってvalueを目的のメモリまたはレジスタに書き込む
 *
 * 引数
 *   emu: エミュレータ構造体(eipはどこを指していても良い)
 *   modrm: ModR/M(SIB, dispを含む)
 *   value: 即値
 */
void set_rm32(Emulator* emu, ModRM* modrm, uint32_t value);

/* r32のレジスタの32bit値を取得する */
uint32_t get_r32(Emulator* emu, ModRM* modrm);

/* r32のレジスタの32bit値を設定する */
void set_r32(Emulator* emu, ModRM* modrm, uint32_t value);

/* 8ビット版 */
uint8_t get_rm8(Emulator* emu, ModRM* modrm);
void set_rm8(Emulator* emu, ModRM* modrm, uint8_t value);
uint8_t get_r8(Emulator* emu, ModRM* modrm);
void set_r8(Emulator* emu, ModRM* modrm, uint8_t value);

#endif
