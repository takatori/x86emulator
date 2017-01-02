#include <stdio.h>
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
