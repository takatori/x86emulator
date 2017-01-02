#include "emulator_function.h"

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
