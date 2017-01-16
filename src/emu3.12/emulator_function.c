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


uint8_t get_register8(Emulator* emu, int index) {
  if (index < 4) {
    return emu->registers[index] & 0xff;
  } else {
    return (emu->registers[index - 4] >> 8) & 0xff;
  }
}

uint32_t get_register32(Emulator* emu, int index) {
  return emu->registers[index];
}

void set_register8(Emulator* emu, int index, uint8_t value) {
  if(index < 4) {
    uint32_t r = emu->registers[index] & 0xffffff00;
    emu->registers[index] = r | (uint32_t)value;
  } else {
    uint32_t r = emu->registers[index - 4] & 0xffff00ff;
    emu->registers[index - 4] = r | ((uint32_t)value << 8);
  }
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

/* 与えられた条件式が真ならキャリーフラグを1にし、偽ならキャリーフラグを0にする関数 */
void set_carry(Emulator* emu, int is_carry) {
  if(is_carry) {
    emu->eflags |= CARRY_FLAG;
  } else {
    emu->eflags &= ~ CARRY_FLAG;
  }
}

void set_zero(Emulator* emu, int is_zero){
  if(is_zero) {
    emu->eflags |= ZERO_FLAG;
  } else {
    emu->eflags &= ~ ZERO_FLAG;
  }  
}

void set_overflow(Emulator* emu, int is_overflow)
{
    if (is_overflow) {
        emu->eflags |= OVERFLOW_FLAG;
    } else {
        emu->eflags &= ~OVERFLOW_FLAG;
    }
}

/* 減算の結果に応じてeflagsのフラグを更新する関数 */
/* 引数v1とv2にはsub命令の2つのオペランドを渡し、resultには減算の結果を渡す */
/* eflagsの中のきゃりーフラグは2つの32ビット値を引き算した結果が桁あふれを起こしているかを表している。
   update_eflags_subの中でそれを判断するためには、計算結果の32ビット目を知る必要があるため、
   resultを64ビット整数としている*/
void update_eflags_sub(Emulator* emu, uint32_t v1, uint32_t v2, uint64_t result) {

  // v1, v2, resultの31ビット目を取り出している
  // 符号つき32ビット整数において31ビット目は符号ビットなので、それが1なら負数、0なら非負数と判断できる
  int sign1 = v1 >> 31;
  int sign2 = v2 >> 31;
  int signr = (result >> 31) & 1;

  // フラグビットの値が1になる条件を計算している

  /* キャリーフラグは減算において32ビット目で繰り下がりが起こったときに1になる必要があり、
     それはresultの32ビット目以降が0でないことと等価である。
     resultの32ビット目以降が0でないときに1、それ以外のときに0になる条件は (result >> 32) != 0 で計算できる。
     C言語では0が偽、非0が真ということになっているので、この条件をif文で使うことを考えると
     きっちり0または1になっている必要はない。0または0でない数値になっていればよいので
     result >> 32 と簡略化できる。*/
  set_carry(emu, result >> 32);
  /* 減算結果が0のときに1になる */
  set_zero(emu, result == 0);
  set_sign(emu, signr);
  /* 減算結果が32ビットに収まりきらないとき1になる */
  /* sign1 != sign2
     2つの符号付き32ビット整数の符号が同じ時、つまり両方共0以上の数か、両方共0未満の数のとき 
     その減算結果は駆らなず符号付き32ビット整数として表せる 
     片方が0以上でもう片方が0未満の数のとき、結果が符号付き32ビットに収まらない場合が出てくる */
  /* sign1 != signr 
   「正」-「負」の場合、すなわち「正」+「正」の場合、その結果が正の最大値を超えてしまうとオーバーフローして結果が負になる
   「負」-「正」の場合、すなわち「負」-「負」の場合、その結果が負の最小値を超えてしまうとオーバーフローして結果が正になる*/
  set_overflow(emu, sign1 != sign2 && sign1 != signr);
}
