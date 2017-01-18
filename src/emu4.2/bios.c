#include "bios.h"

#include <stdio.h>
#include <stdint.h>
#include "emulator_function.h"
#include "io.h"

/* BIOSの色コードを端末の色コードに変換するテーブル */
static int bios_to_terminal[8] = {30, 34, 32, 36, 31, 35, 33, 37};

static void put_string(const char* s, size_t n) {
  size_t i;
  for(i = 0; i < n; i++) {
    io_out8(0x03f8, s[i]); // 0x03f8番のポートに１バイト出力すると、それがそのまま画面に出力される
  }
}

/* alレジスタに格納された文字コードを、blレジスタで指定された文字色で画面に印字する */
static void bios_video_teletype(Emulator* emu){

  /* BIOSの一文字表示機能ではblレジスタに文字色を指定する*/
  uint8_t color = get_register8(emu, BL) & 0x0f;
  uint8_t ch    = get_register8(emu, AL);

  char buf[32];
  /* blレジスタでBIOSに指定できる色は4ビットで、そのうち最上位ビットは輝度を表す */
  /* 下位3ビットを取り出してBIOSの色番号からANSIエスケープシーケンスの色番号へ変換した値をterminal_colorに書き込む*/
  int terminal_color = bios_to_terminal[color & 0x07];
  int bright         = (color & 0x08) ? 1 : 0;
  /* ANSIエスケープシーケンスによる色付け */
  /* \x1b[1;32m 明るい緑色にする*/
  /* \x1b[0m 文字色のリセット*/
  /* \x1b[輝度;色番号m文字コード\x1b[0m */
  /* C言語の機能で\xYYと書くと文字コードがYYである一文字を表すことができる。
     画面に表示できないような特殊な文字を文字列の一部として埋め込むために使える機能*/
  int len            = sprintf(buf, "\x1b[%d;%dm%c\x1b[0m", bright, terminal_color, ch);
  put_string(buf, len);
}


void bios_video(Emulator* emu) {
  /* BIOSの機能はまず、割り込み番号で大雑把に分類され、
     次にahレジスタの値で細かい機能が決まる */
  uint8_t func = get_register8(emu, AH);
  
  switch(func) {
  case 0x0e: // 一文字表示機能(テレタイプ出力)
    bios_video_teletype(emu);
    break;
  default:
    printf("not implemented BIOS video function: 0x%02x\n", func);      
  }
}
