CC = gcc
LD = gcc

SRC_PATH = ./src/
INCLUDE_PATH = ./include/

SRCS = $(wildcard $(SRC_PATH)*.c)
INCLUDE = -I$(INCLUDE_PATH)
LIBS = -l pthread#-L 
OBJS = $(patsubst %c, %o, $(SRCS))

CFLAGS += -g -Wall -O2

TARGET = ringbuffer

.PHONY:all clean


all:
	make clean
	make $(TARGET)

$(TARGET):$(OBJS)
	$(LD) -o $@ $^ $(LIBS)
%.o:%.c
	$(CC) $(INCLUDE) $(CFLAGS) $(LIBS) -c $< -o $@


clean:
	rm -rf $(TARGET) $(OBJS)
