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

        virtual uint32_t GetNodeId() = 0;

        virtual uint16_t GetPort() = 0;

        virtual std::string GetIp() = 0;

        virtual std::string GetLogFile() = 0;

        virtual bool PrintLog() = 0;

        virtual bool GetThreadNum() = 0;

        virtual uint16_t GetLogLevel() = 0;

        virtual std::string GetNodeInfo() = 0;
    };
}

#endif