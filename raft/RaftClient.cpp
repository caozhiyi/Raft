#include <memory>
#include "RaftClient.h"
#include "RaftClientImpl.h"

void raft::Init(const std::string& config_file) {
    CRaftClientImpl::Instance().Init(config_file);
}

void raft::Dealloc() {
    CRaftClientImpl::Instance().Dealloc();
}

void raft::Join() {
    CRaftClientImpl::Instance().Join();
}

void raft::SendEntries(const std::string& entries) {
    CRaftClientImpl::Instance().SendEntries(entries);
}

void raft::SetCommitEntriesCallBack(std::function<void(ERR_CODE)> func) {
    CRaftClientImpl::Instance().SetCommitEntriesCallBack(func);
}