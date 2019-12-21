#include "absl/strings/str_format.h"
#include "Log.h"
#include "Node.h"
#include "IConfig.h"
#include "RaftTimer.h"
#include "CppnetImpl.h"
#include "LeaderRole.h"
#include "NodeManager.h"
#include "RaftMediator.h"
#include "FollowerRole.h"
#include "CandidateRole.h"
#include "ClientManager.h"
#include "CommitEntriesDisk.h"

using namespace raft;

CRaftMediator::CRaftMediator() {
    
    // create timer
    _timer = std::make_shared<CTimerImpl>();

    // creata role data
    std::shared_ptr<CRoleData> data(new CRoleData);
    // create all role control 
    _leader_role.reset(new CLeaderRole(data, _timer, this));
    _candidate_role.reset(new CCandidateRole(data, _timer, this));
    _follower_role.reset(new CFollowerRole(data, _timer, this));

    // create net 
    _net.reset(new CCppNet());
    // create node manager
    _node_manager.reset(new CNodeManagerImpl(_net));
    // create client manager
    _client_manager.reset(new CClientManagerImpl(_net));

    // set current role to follower
    ChangeRole(follower_role);

    // create commit entries
    std::string file = _config->GetCommitDiskFile();
    _commit_entries.reset(new CCommitEntriesDisk(file));
}

CRaftMediator::~CRaftMediator() {

}

uint32_t CRaftMediator::GetId() {
    return _id;
}

void CRaftMediator::CommitEntries(Entries& entries) {
    if (_commit_call_back) {
        _commit_call_back(entries._entries);
    }
    
    _commit_entries->PushEntries(entries);
}

void CRaftMediator::ChangeRole(ROLE_TYPE type) {
    base::LOG_DEBUG("change role to %d", type);
    if (type == follower_role) {
        _current_role = _follower_role;

    } else if (type == candidate_role) {
        _current_role = _candidate_role;

    } else if (type == leader_role) {
        _current_role = _leader_role;

    } else {
        base::LOG_ERROR("unknow role type.");
    }
    _node_manager->SetRole(_current_role);
    _client_manager->SetRole(_current_role);
}

void CRaftMediator::SendVoteToAll(VoteRequest& request) {
    _node_manager->SendVoteToAll(request);
}

void CRaftMediator::SendHeartBeatToAll(HeartBeatResquest& request) {
    _node_manager->SendHeartToAll(request);
}

void CRaftMediator::RecvHeartBeat(std::shared_ptr<CNode>& node, HeartBeatResquest& request) {
    _current_role->RecvHeartBeatRequest(node, request);
}

void CRaftMediator::SetCommitCallBack(absl::FunctionRef<void(std::string)> func) {
    _commit_call_back = func;
}