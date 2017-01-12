#ifndef EMULATOR_FUNCTION_H_
#define EMULATOR_FUNCTION_H_

#include <stdint.h>

#include "emulator.h"

/* EFLAGSのビットフラグ */
#define CARRY_FLAG (1)
#define ZERO_FLAG (1 << 6)
#define SIGN_FLAG (1 << 7)
#define OVERFLOW_FLAG (1 << 11)

/* プログラムカウンタから相対位置にある符号無し8bit値を取得 */
uint32_t get_code8(Emulator* emu, int index);

/* プログラムカウンタから相対位置にある符号付き8bit値を取得 */
int32_t get_sign_code8(Emulator* emu, int index);

/* プログラムカウンタから相対位置にある符号無し32bit値を取得 */
uint32_t get_code32(Emulator* emu, int index);

/* プログラムカウンタから相対位置にある符号付き32bit値を取得 */
int32_t get_sign_code32(Emulator* emu, int index);

/* index番目の32bit汎用レジスタの値を取得する */
uint32_t get_register32(Emulator* emu, int index);

/* index番目の32bit汎用レジスタに値を設定する */
void set_register32(Emulator* emu, int index, uint32_t value);

/* メモリのindex番地の8bit値を取得する */
uint32_t get_memory8(Emulator* emu, uint32_t address);

/* メモリのindex番地の32bit値を取得する */
uint32_t get_memory32(Emulator* emu, uint32_t address);

/* メモリのindex番地に8bit値を設定する */
void set_memory8(Emulator* emu, uint32_t address, uint32_t value);

/* メモリのindex番地に32bit値を設定する */
void set_memory32(Emulator* emu, uint32_t address, uint32_t value);

/* スタックに32bit値を積む */
void push32(Emulator* emu, uint32_t value);

/* スタックから32bit値を取り出す */
uint32_t pop32(Emulator* emu);

/* EFLAGの各フラグ設定用関数 */
void set_carry(Emulator* emu, int is_carry);
void set_zero(Emulator* emu, int is_zero);
void set_sign(Emulator* emu, int is_sign);
void set_overflow(Emulator* emu, int is_overflow);

/* EFLAGの各フラグ取得用関数 */
int32_t is_carry(Emulator* emu);
int32_t is_zero(Emulator* emu);
int32_t is_sign(Emulator* emu);
int32_t is_overflow(Emulator* emu);

/* 減算によるEFLAGSの更新関数 */
void update_eflags_sub(Emulator* emu, uint32_t v1, uint32_t v2, uint64_t result);

#endif
