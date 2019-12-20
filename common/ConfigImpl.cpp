#include "ConfigImpl.h"

using namespace raft;

CConfigImpl::CConfigImpl(const std::string& config_file) {
    _config.SetFilePath(config_file);
    _config.ReLoadFile();
}

CConfigImpl::~CConfigImpl() {

}

std::string CConfigImpl::GetCommitDiskFile() {
    return _config.GetStringValue("commit_file");
}

std::pair<uint32_t, uint32_t> CConfigImpl::GetVoteTimerRandomRange() {
    std::pair<uint32_t, uint32_t> ret;
    ret.first = _config.GetIntValue("vote_timer_start");
    ret.second = _config.GetIntValue("vote_timer_end");
    return std::move(ret);
}