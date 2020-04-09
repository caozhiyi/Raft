#include "server.h"

int main() {
    CServer server;

    server.Init("./raft.conf");
    server.Join();

    return 0;
}