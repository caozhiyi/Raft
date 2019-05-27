SRCS = $(wildcard ./base/*.cpp ./exe/*.cpp ./raft/*.cpp ./server/*.cpp ./pb/*.cc)
PB=$(wildcard ./pb/*.proto)
PROTO=/usr/bin/protoc
OBJS = $(SRCS:.c* = .o)

CC = g++

INCLUDES = -I.               \
           -I./base          \
           -I./brpc/include  \
           -I./server        \
           -I./raft          \
           -I./pb            \

LIBS = -L./brpc/lib

CCFLAGS = -lpthread -fPIC -m64 -std=c++11 -lstdc++ -fpermissive -lbrpc -lprotobuf

OUTPUT = raft_brpc

all:$(OUTPUT)

$(OUTPUT) : $(OBJS)
	${PROTO} -I=./pb --cpp_out=./pb  ${PB}
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS)

%.o : %.c
	$(CC) -c $< $(CCFLAGS)

clean:
	rm -rf *.out *.o ${OUTPUT} ./pb/*.h ./pb/*.cc

.PHONY:clean
