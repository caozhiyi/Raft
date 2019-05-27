#ifndef HEADER_COMMON
#define HEADER_COMMON

namespace raft {
    typedef unsigned long long Time;
    // node role
    enum NodeRole {
        Follower = 1,
        Candidate = 2,
        Leader = 3
    };

    enum ErrorCode {
        ERR_Success = 0,
        ERR_LessNodeRecv = 1,
        ERR_NotLeader = 2
    };
}
#endif