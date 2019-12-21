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


uint32_t CConfigImpl::GetNodeId() {
    return _config.GetIntValue("node_id");
}

uint16_t CConfigImpl::GetPort() {
    return _config.GetIntValue("port");
}

std::string CConfigImpl::GetIp() {
    return _config.GetStringValue("local_ip");
}

std::string CConfigImpl::GetLogFile() {
    return _config.GetStringValue("log_file");
}

bool CConfigImpl::PrintLog() {
    return _config.GetBoolValue("print_log");
}

bool CConfigImpl::GetThreadNum() {
    return _config.GetIntValue("thread_num");
}

uint16_t CConfigImpl::GetLogLevel() {
    return _config.GetIntValue("log_level");
}

std::string CConfigImpl::GetNodeInfo() {
    return _config.GetStringValue("node_info");
}