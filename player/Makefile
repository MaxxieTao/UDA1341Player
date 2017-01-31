DIR_INC = ./include
DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./bin

KERN_DIR = /work/system/linux-2.6.22.6
CC = arm-linux-gcc
CFLAGS = -I$(DIR_INC)
LDFLAGS := -lm -lpthread
	
#目标文件名
TARGET = main
#BIN_TARGET = ${DIR_BIN}/${TARGET}

#获取./src/app下所有.c文件
APP = $(wildcard ${DIR_SRC}/*.c)
#将.c文件名替换为.o   即应对的obj文件
OBJ = $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${APP})) 
all:$(DIR_BIN)/$(TARGET) drivers

drivers:
	${MAKE} -C $(KERN_DIR) M=$(PWD)/driver modules 
	
${DIR_BIN}/${TARGET}:$(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)
	
${DIR_OBJ}/%.o:$(DIR_SRC)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
.PHONY:clean
clean:
	rm -f $(DIR_OBJ)/*
	rm -f $(DIR_BIN)/*
	make -C $(KERN_DIR) M=$(PWD)/driver modules clean

