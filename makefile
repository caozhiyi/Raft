SRCS = $(wildcard ./base/*.cpp ./common/*.cpp ./pb/*.cpp ./raft/*.cpp)

OBJS = $(patsubst %.cpp, %.o, $(SRCS))


CC = g++

INCLUDES = -I.                 \
           -I./base            \
		   -I./pb              \
		   -I./raft            \
           -I./common          \
           -I./include         \
           -I./interface       \
		   -I./third           \
		   -I./third/CppNet    \
		   -I./third/protobuf  \


#debug
#CCFLAGS = -lpthread -fPIC -m64 -g -pg -std=c++11 -lstdc++ -pipe 

CCFLAGS = -lpthread -fPIC -m64 -O2 -std=c++11 -lstdc++ -pipe -march=corei7 

TARGET = libraft.a

all:$(TARGET)

$(TARGET):$(OBJS)
	ar rcs $@ $^

%.o : %.cpp
	$(CC) -c $< -o $@ $(CCFLAGS) $(INCLUDES) 

clean:
	rm -rf $(OBJS) $(TARGET) $(SERBIN) $(CLIBIN)