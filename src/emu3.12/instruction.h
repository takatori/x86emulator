#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include "emulator.h"

/* 関数プロトタイプ宣言 */
/* 「init_instructionsという名前の関数は引数も戻り値もありません」という意味 */
/* 関数本体はここにない */
void init_instructions(void);

typedef void instruction_func_t(Emulator*);

/* 変数のextern宣言 */
/* 「ここではinstructios配列の実体(メモリ領域)はないけど、どこかにあるはずですよ」という意味 */
/* x86命令の配列、opecode番目の関数がx86のopecodeに対応した命令となっている */
extern instruction_func_t* instructions[256];

#endif
