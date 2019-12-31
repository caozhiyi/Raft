#include "server.h"

int main() {
    CServer server;
    
    server.Init("E:/Github/Raft/conf/raft.conf");
    server.Join();

    return 0;
}