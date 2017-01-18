BITS 32
  org 0x7c00
start:                          ; プログラムの開始
  mov esi, msg                  
  call puts                     ; サブルーチンを呼び出す
  jmp 0                         ; プログラムの終了

puts:
  mov al, [esi]                 ; 1文字読み込む
  inc esi
  cmp al, 0                     ; 文字列の末尾
  je puts_end                   ; に来たら終了
  mov ah, 0x0e                  ; 1文字表示機能
  mov ebx, 10                   ; 文字色の指定
  int 0x10                      ; BIOSを呼び出す intのオペランドとahレジスタの組み合わせでBIOSが持つ機能群のどれを呼び出すかを選択
  jmp puts
puts_end:
  ret                           ; サブルーチンから抜ける

msg:
  db "hello, world", 0x0d, 0x0a, 0

