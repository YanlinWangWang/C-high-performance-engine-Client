
# 指令编译器和选项
CC=g++
CFLAGS=-pthread
 
# 目标文件
TARGET=client
SRCS = client.cpp \

 #头文件
INC = -I./EasyTcpClient -I./MessageHeader
 
OBJS = $(SRCS:.cpp=.o)
 
$(TARGET):$(OBJS)
#	@echo TARGET:$@
#	@echo OBJECTS:$^
	$(CC) -o $@ $^ $(CFLAGS)
 
clean:
	rm -rf $(TARGET) $(OBJS)
 
%.o:%.cpp
	$(CC)  $(INC) -o $@ -c $< $(CFLAGS)
