#ifndef HEADER_COMMON
#define HEADER_COMMON

namespace raft {
    const int rpc_time_out = 500; //ms
    const std::string rpc_server_version = "raft_server_1.0";
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