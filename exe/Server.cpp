#include "CNode.h"
#include "Log.h"
#include "UnixOs.h"
using namespace std;

int main() {
    SetCoreFileUnlimit();
    CLog::Instance().SetLogName("server.log");
    CLog::Instance().SetLogLevel(LOG_ERROR_LEVEL);
    CLog::Instance().Start();

    raft::CNode node("server.conf");
    node.Init();

    return 0;
}     
