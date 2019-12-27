SRCS = $(wildcard ./http/*.cpp ./raft/*.cpp ./*.cpp)

OBJS = $(patsubst %.cpp, %.o, $(SRCS))


CC = g++

INCLUDES = -I./http            	   \
		   -I./raft            	   \
		   -I../../base            \
           -I../../include         \
		   -I../../third           \
		   -I../../third/CppNet	   \
		   -I../../third/protobuf  \


#debug
#CCFLAGS = -lpthread -fPIC -m64 -g -pg -std=c++11 -lstdc++ -pipe 

CCFLAGS = -lpthread -fPIC -m64 -O2 -std=c++11 -lstdc++ -pipe -march=corei7 

TARGET = ../../libraft.a
SERBIN = raftser

all: $(SERBIN)

$(SERBIN):$(OBJS) $(TARGET)
	$(CC) $^ -o $@  $(TARGET)  $(CCFLAGS) $(INCLUDES)

%.o : %.cpp
	$(CC) -c $< -o $@ $(CCFLAGS) $(INCLUDES) 

clean:
	rm -rf $(OBJS) $(SERBIN)