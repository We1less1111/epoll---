TARGET = myserver

# c源文件

SRC = myserver.c  ae.c ae_epoll.c  networking.c  zmalloc.c anet.c 

# object文件

OBJS = $(SRC:.c=.o)

# 编译器

CC = gcc

# 头文件路径，可以任意指定头文件

INCLUDE = -I/usr/include/ -I/home/ubuntu/Documents/mydesign

# 链接库

LINKPARAM = -lpthread 

# 编译选项

CFLAGS = -ansi -g -w

 

all:$(TARGET)

# 定义目标文件生成规则

$(TARGET):$(OBJS)

	$(CC) -o $(TARGET) $(LINKPARAM) $(OBJS)

 

.SUFFIXES:.c

# 定义obejct文件生成规则

.c.o:

	$(CC) $(LINKPARAM) $(INCLUDE) $(CFLAGS) -c $<

# 定义清理函数

clean:

	rm $(OBJS) $(TARGET)

