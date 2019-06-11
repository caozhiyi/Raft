#include "CNode.h"
#include "Log.h"
#include "UnixOs.h"
using namespace std;

int main() {
    SetCoreFileUnlimit();

    raft::CNode node("server.conf");
    node.Init(LOG_ERROR_LEVEL);

    return 0;
}     
