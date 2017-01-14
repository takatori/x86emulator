/* インクルードガード */

/* if Not Definedの意味　
   EMULATOR_H_というマクロが定義されていなければ、
   対応する#endifまでの行が有効化される
   すでに存在していれば、それらの行はプリプロセッサにより無視される*/
#ifndef EMULATOR_H_
#define EMULATOR_H_

#include <stdint.h>

enum Register { EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, REGISTERS_COUNT };

typedef struct {
  /* 汎用レジスタ　*/
  uint32_t registers[REGISTERS_COUNT];

  /* EFLAGSレジスタ */
  uint32_t eflags;
  
  /* メモリ(バイト列) */
  uint8_t* memory;

  /* プログラムカウンタ */
  /* 実行中の機械語が置いてあるメモリ番地を記憶するレジスタ */
  uint32_t eip;
} Emulator;

#endif
