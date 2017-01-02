#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* メモリは1MB */
#define MEMORY_SIZE (1024 * 1024)

enum Register { EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI, REGISTERS_COUNT };
char* registers_name[] = {
  "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"};

typedef struct {
  /* 汎用レジスタ　*/
  uint32_t registers[REGISTERS_COUNT];

  /* EFLAGSレジスタ */
  uint32_t eflags;
  
  /* メモリ(バイト列) */
  uint8_t* memory;

  /* プログラムカウンタ */
  /* 実行中の機械語が置いてあるメモリ番地を記憶するレジスタ */
  uint32_t eip;
} Emulator;


/* エミュレータを作成する */
Emulator* create_emu(size_t size, uint32_t eip, uint32_t esp) {

  Emulator* emu = malloc(sizeof(Emulator));
  emu->memory   = malloc(size);

  /* 汎用レジスタの初期値をすべて0にする */
  memset(emu->registers, 0, sizeof(emu->registers));

  /* レジスタの初期値を指定されたものにする */
  emu->eip            = eip;
  emu->registers[ESP] = esp;

  return emu;
  
}

/* エミュレータを破棄する */
void destroy_emu(Emulator* emu) {
  free(emu->memory);
  free(emu);
}


/* 汎用時レスタとプログラムカウンタの値を標準出力に出力する */
static void dump_registers(Emulator* emu) {
  
  int i;

  for (i = 0; i < REGISTERS_COUNT; i++) {
    printf("%s = %08x\n", registers_name[i], emu->registers[i]);
  }

  printf("EIP = %08x\n", emu->eip);
  
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

/* 汎用レジスタに32ビットの即値をコピーするmov命令に対応する */
void mov_r32_imm32(Emulator* emu) {
  /* このmov命令のオペコードはrをレジスタ番号だとすると0xb8+r */
  /* オペコード自身がレジスタの指定を含むタイプの命令 */
  uint8_t  reg   = get_code8(emu, 0) - 0xB8;
  /* オペコードのすぐ後に32ビットの即値がくるはずなので、
     get_code32で32ビット値を読み取ってレジスタに代入している */
  uint32_t value = get_code32(emu, 1);

  emu->registers[reg] = value;
  emu->eip += 5;  
}

/* 1バイトのメモリ番地を取るjump命令、ショートジャンプ命令に対応する */
/* この命令はオペランドにバイトの符号付き整数(つまり2の補数表現で解釈される)を取りeipに加算する
   したがって現在地から前に127バイト、後ろに128バイトの範囲内でジャンプすることができる */
void short_jump(Emulator* emu) {
  /* オペランドを8ビット符号付き整数としてdiffに読み込む */
  int8_t diff = get_sign_code8(emu, 1);
  /* jump命令はその次の命令の番地を起点にjump先を計算するので、
     eipにはdiff + 2(ショートジャンプ命令は2バイト命令)を加算する*/
  emu->eip += (diff + 2);
}

/* 32ビットの符号つき整数を取る相対ジャンプ命令 */
void near_jump(Emulator* emu) {
  int32_t diff = get_sign_code32(emu, 1);
  emu->eip += (diff + 5);
}


typedef void instruction_func_t(Emulator*);
instruction_func_t* instructions[256];
void init_instructions(void) {
  int i;
  memset(instructions, 0, sizeof(instructions));
  for(i = 0; i < 8; i++) {
    instructions[0xB8 + i] = mov_r32_imm32;
  }
  instructions[0xE9] = near_jump;
  instructions[0xEB] = short_jump;  
}

/* 
エミュレータ構造体を生成して初期化し、ファイルから機械語プログラムを読み込む処理 
コマンドライン引数に機械語プログラムが格納されたファイルを指定する
*/
int main(int args, char* argv[]) {

  FILE* binary;
  Emulator* emu;

  /* コマンドライン引数が一つ指定されていることを確認 */
  if(args != 2) {
    printf("usage: x86 filename\n");
    return 1;
  }

  /* EIPが0、ESPが0x7C00の状態のエミュレータを作る */
  /* 左からメモリ容量、eipの初期値、espの初期値 */
  emu = create_emu(MEMORY_SIZE, 0x7c00, 0x7c00);

  binary = fopen(argv[1], "rb");
  if(binary == NULL) {
    printf("%sファイルが開けません\n", argv[1]);
    return 1;
  }

  /* 機械語ファイルを読み込む(最大512バイト) */
  /* memoryの先頭ではなく0x7c00番地から機械語を配置する */
  /* この番地とeipの初期値、orgの値の３つの値が一致していて初めて正しくプログラムが動作する */
  fread(emu->memory + 0x7c00, 1, 0x200, binary);
  fclose(binary);


  init_instructions();

  while(emu->eip < MEMORY_SIZE) {
    uint8_t code = get_code8(emu, 0);
    if(instructions[code] == NULL) {
      printf("\n\nNot Implemented: %x\n", code);
      break;
    }
    
    /* 命令の実行 */
    instructions[code](emu);
    /* 一つの命令を実行するたびにeipをチェックし、0ならメインループを終了する */
    /* 普通のCPUには終了機能はないが、エミュレータではプログラムの修了時にレジスタの値を表示したいので、明示的に終了させる仕組みが必要 */
    if(emu->eip == 0x00) {
      printf("\n\nend of program. \n\n");
      break;
    }
    
  }


  dump_registers(emu);
  destroy_emu(emu);
  return 0;  
}
