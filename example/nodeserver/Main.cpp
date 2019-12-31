#include "server.h"

int main() {
    CServer server;
    
    server.Init("./conf/raft.conf");
    server.Join();

    return 0;
}