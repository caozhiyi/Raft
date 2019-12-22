#include <memory>
#include "RaftClient.h"
#include "ClientRaftImpl.h"

void raft::Init(const std::string& config_file) {
    CClientRaftImpl::Instance().Init(config_file);
}

void raft::Dealloc() {
    CClientRaftImpl::Instance().Dealloc();
}

void raft::Join() {
    CClientRaftImpl::Instance().Join();
}

void raft::SendEntries(const std::string& entries) {
    CClientRaftImpl::Instance().SendEntries(entries);
}

void raft::SetCommitEntriesCallBack(std::function<void(ERR_CODE)> func) {
    CClientRaftImpl::Instance().SetCommitEntriesCallBack(func);
}