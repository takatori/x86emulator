BITS 32                         ; アセンブラNASMに対して32ビットモードでアセンブルする旨を伝える
start:                          ; ラベル。その地点のメモリ番地に名前をつける
  mov eax, 41                   ; eaxレジスタに41を代入する
  jmp short start               ; startラベルにジャンプする
