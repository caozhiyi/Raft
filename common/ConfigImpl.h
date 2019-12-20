#ifndef RAFT_COMMON_CONFIGIMPL
#define RAFT_COMMON_CONFIGIMPL

#include "IConfig.h"
#include "Config.h"

namespace raft {

    class CConfigImpl {
    public:
        CConfigImpl(const std::string& config_file);
        virtual ~CConfigImpl();

        std::string GetCommitDiskFile();

        std::pair<uint32_t, uint32_t> GetVoteTimerRandomRange();

    private:
        base::CConfig   _config;
    };
}

#endif