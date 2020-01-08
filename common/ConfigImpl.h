#ifndef RAFT_COMMON_CONFIGIMPL
#define RAFT_COMMON_CONFIGIMPL

#include "IConfig.h"
#include "Config.h"

namespace raft {

    class CConfigImpl : public CConfig  {
    public:
        CConfigImpl(const std::string& config_file);
        ~CConfigImpl();

        std::string GetCommitDiskFile();

        std::pair<uint32_t, uint32_t> GetVoteTimerRandomRange();

        uint32_t GetNodeId();

        uint16_t GetPort();

        std::string GetIp();

        std::string GetLogFile();

        bool PrintLog();

        bool GetThreadNum();

        uint16_t GetLogLevel();

        std::string GetNodeInfo();

        uint32_t GetHeartTime();

        bool IsWithClient();

    private:
        base::CConfig   _config;
    };
}

#endif