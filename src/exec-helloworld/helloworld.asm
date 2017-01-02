BITS 32
  org 0x7c00                    ; org疑似命令でプログラムの配置場所をBIOSと同じ0x7c00番地に配置する
  mov eax, 41
  jmp 0
