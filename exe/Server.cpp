#include <fcntl.h>
#include <sys/resource.h>
#include <sys/socket.h>

#include "CNode.h"
#include "Log.h"

using namespace std;

void SetCoreFileUnlimit() {
    struct rlimit rlim;
    struct rlimit rlim_new;
    if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
        rlim_new.rlim_cur = rlim_new.rlim_max = RLIM_INFINITY;
        if (setrlimit(RLIMIT_CORE, &rlim_new) != 0) {
            rlim_new.rlim_cur = rlim_new.rlim_max = rlim.rlim_max;
            setrlimit(RLIMIT_CORE, &rlim_new);
        }
    }
}

int main() {
    SetCoreFileUnlimit();
    CLog::Instance().SetLogName("server.log");
    CLog::Instance().SetLogLevel(LOG_DEBUG_LEVEL);
    CLog::Instance().Start();

    raft::CNode node("server.conf");
    node.Init();

    return 0;
}