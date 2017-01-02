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

#endif
