#ifndef RAFT_INTERFACE_CONFIG
#define RAFT_INTERFACE_CONFIG

#include <string>

namespace raft {

    class CConfig {
    public:
        CConfig() {}
        virtual ~CConfig() {}

        virtual std::string GetCommitDiskFile() = 0;

        virtual std::pair<uint32_t, uint32_t> GetVoteTimerRandomRange() = 0;
    };
}

#endif