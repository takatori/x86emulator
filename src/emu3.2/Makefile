TARGET = emu.x86_64
# オブジェクトファイル名を “objs” というマクロとして定義
# “$(objs)” は “main.o” に置換される
OBJS = main.o
CC = gcc
CFLAGS += -Wall

.PHONY: all
all :
	make $(TARGET)

# “$<” という内部マクロを使用している. これは依存ファイルの先頭のファイル名を表す
%.o : %.c Makefile
	$(CC) $(CFLAGS) -c $<


# “$@” という内部マクロを使用している. これはターゲット名を表す
$(TARGET) : $(OBJS) Makefile
	$(CC) -o $@ $(OBJS)
