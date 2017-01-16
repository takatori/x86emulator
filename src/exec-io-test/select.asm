BITS 32
    org 0x7c00
start:
    mov edx, 0x03f8
mainloop:
    mov al, '>'    ; プロンプトを表示
    out dx, al
input:
    in al, dx       ; 1文字入力
    cmp al, 'h'
    je puthello     ; hならhelloを表示
    cmp al, 'w'
    je putworld     ; wならworldを表示
    cmp al, 'q'
    je fin          ; qなら終了
    jmp input       ; それ以外なら再入力
puthello:
    mov esi, msghello
    call puts
    jmp mainloop
putworld:
    mov esi, msgworld
    call puts
    jmp mainloop
fin:
    jmp 0

; esiに設定された文字列を表示するサブルーチン
puts:
    mov al, [esi]
    inc esi
    cmp al, 0
    je putsend
    out dx, al
    jmp puts
putsend:
    ret

msghello:
    db "hello", 0x0d, 0x0a, 0
msgworld:
    db "world", 0x0d, 0x0a, 0
