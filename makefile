SRCS = $(wildcard ./base/*.cpp  ./raft/*.cpp ./server/*.cpp ./pb/*.cc)
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

TAR1 = raft_sev
TAR2 = raft_cli

all: protoc $(TAR1) ${TAR2}

protoc:
	${PROTO} -I=./pb --cpp_out=./pb  ${PB}


$(TAR1) : $(OBJS) ./exe/Server.cpp
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS)
	-mkdir out_put 
	-mv ${TAR1} out_put
	-cp conf/server.conf out_put

${TAR2} : $(OBJS) ./exe/Client.cpp
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS)
	-mkdir out_put 
	-mv ${TAR2}  out_put
	-cp conf/client.conf out_put


%.o : %.c
	$(CC) -c $< $(CCFLAGS)

clean:
	rm -rf *.out *.o ${TAR1} ${TAR2}  out_put

.PHONY:clean
