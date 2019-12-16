#ifndef RAFT_INTERFACE_CONFIG
#define RAFT_INTERFACE_CONFIG

namespace raft {

    class CConfig {
    public:
        CConfig() {}
        virtual ~CConfig() {}

        virtual std::string GetCommitDiskFile();

        virtual std::pair<uint32_t, uint32_t> GetVoteTimerRandomRange();
    };
}

#endif