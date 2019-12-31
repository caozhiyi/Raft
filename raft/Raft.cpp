#include "Raft.h"
#include "RaftMediator.h"

void raft::Init(const std::string& config_file) {
    CRaftMediator::Instance().Start(config_file);
}

void raft::Dealloc() {
    CRaftMediator::Instance().Dealloc();
}

void raft::Join() {
    CRaftMediator::Instance().Join();
}

void raft::SetCommitEntriesCallBack(std::function<void(std::string)> func) {
    CRaftMediator::Instance().SetCommitCallBack(func);
}