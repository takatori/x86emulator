#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "modrm.h"
#include "emulator_function.h"

/* 機械語からModR/Mを解析する関数 */
/* エミュレータ内部の状態を格納しているEmulator構造体と、
   ModR/Mの解析結果を格納するためのModRM構造体を引数にとり、
   emu->eipが指すメモリ領域からModR/MとSIBとディプレースメントを読み取る。
   この関数を呼び出すときは、emu->memory[emu->eip]がModR/Mバイトを指している状態でなければならない
 */
void parse_modrm(Emulator* emu, ModRM* modrm) {
  uint8_t code;

  memset(modrm, 0, sizeof(ModRM)); // ModRM構造体の全部を0に初期化

  // ModR/Mバイトの各ビットを取り出してmod,opecode,rmに書き込み
  code = get_code8(emu, 0); 
  modrm->mod     = ((code & 0xC0) >> 6); // 6,7ビットのみ抽出
  modrm->opecode = ((code & 0x38) >> 3); // 3,4,5ビットのみ抽出
  modrm->rm      = code & 0x07;          // 0,1,2ビットのみ抽出

  // emu->eipを１バイト進める
  emu->eip += 1;

  // SIBの存在を判定
  // ModR/Mの表を見ると、modが11以外であり、かつrmが100であるときにSIBが存在する
  if(modrm->mod != 3 && modrm->rm == 4) {
    // SIBが存在するなら、読み取ってsibに書き込み、emu->eipを進める
    modrm->sib = get_code8(emu, 0);
    emu->eip += 1;
  }

  // ディプレースメントの有無を判定し、ビット幅に応じてdisp8またはdisp32に書き込みeipを進める
  if((modrm->mod == 0 && modrm->rm == 5) || modrm->mod == 2) {
    modrm->disp32 = get_sign_code32(emu, 0);
    emu->eip += 4;
  } else if(modrm->mod == 1) {
    modrm->disp8 = get_sign_code8(emu, 0);    
    emu->eip += 1;
  }
}

/* メモリ番地の計算を行う */
/* modが3以外の場合の書き込み先はメモリ領域で、番地の表し方は[eax]だったり[ebp]+disp8だったりとさまざまある */
uint32_t calc_memory_address(Emulator* emu, ModRM* modrm) {
  if(modrm->mod == 0) {
    if(modrm->rm == 4) {
      printf("not implemented ModRM mod = 0, rm = 4\n");
      exit(0);
    } else if(modrm->rm == 5) {
      return modrm->disp32;
    } else {
      return get_register32(emu, modrm->rm);
    } 
  } else if(modrm->mod == 1) {
    if(modrm->rm == 4) {
      printf("ont implemented ModRM mod = 1, rm = 4\n");
      exit(0);
    } else {
      /* modが1でrmが4以外のときの計算式は[レジスタ]+disp8であり、[ebp-4]に該当する */
      /* rmにレジスタ番号が入っているのでget_register32でレジスタの値を取ってきて、
         そこにディプレースメントを足した値が最終的なメモリ番地になる */
      return get_register32(emu, modrm->rm) + modrm->disp8;
    }
  } else if(modrm->mod == 2) {
    if(modrm->rm == 4) {
      printf("not implemented ModRM mod = 2, rm = 4\n");
      exit(0);
    } else {
      return get_register32(emu, modrm->rm) + modrm->disp32;
    }
  } else {
    printf("not implemented ModRM mod = 3\n");
    exit(0);
  }
}

/* rm32(modrmの値によって指定されるレジスタまたはメモリ領域)に、valueで指定された32ビット値を書き込む*/
void set_rm32(Emulator* emu, ModRM* modrm, uint32_t value) {

  /* modが3のときは書き込み先はレジスタ */
  /* rmがそのままレジスタ番号を指す */
  if(modrm->mod == 3) {
    set_register32(emu, modrm->rm, value);
  } else {
    /* 書き込み先はメモリ領域 */
    uint32_t address = calc_memory_address(emu, modrm);
    set_memory32(emu, address, value);
  }
}

/* modとrmで指定した場所から32ビット値を読み込む関数 */
uint32_t get_rm32(Emulator* emu, ModRM* modrm) {

  if(modrm->mod == 3) {
    return get_register32(emu, modrm->rm);
  } else {
    uint32_t address = calc_memory_address(emu, modrm);
    return get_memory32(emu, address);
  }
}

void set_r32(Emulator* emu, ModRM* modrm, uint32_t value) {
  set_register32(emu, modrm->reg_index, value);
}

uint32_t get_r32(Emulator* emu, ModRM* modrm) {
  return get_register32(emu, modrm->reg_index);
}
