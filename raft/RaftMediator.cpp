#include "RaftMediator.h"
#include "NodeImpl.h"
#include "absl/strings/str_format.h"
#include "Log.h"
#include "CommitEntriesDisk.h"
#include "LeaderRole.h"
#include "CandidateRole.h"
#include "FollowerRole.h"
#include "Timer.h"
#include "CommitEntriesDisk.h"
#include "IConfig.h"

using namespace raft;

CRaftMediator::CRaftMediator() {
    
    // create timer
    _timer.reset(new CTimerImpl());

    // creata role data
    std::shared_ptr<CRoleData> data(new CRoleData);
    // create all role control 
    _leader_role.reset(new CLeaderRole(data, _timer, this));
    _candidate_role.reset(new CCandidataRole(data, _timer, this));
    _follower_role.reset(new CFollowerRole(data, _timer, this));

    // set current role to follower
    _current_role = _follower_role;

    // create commit entries
    std::string file = _config->GetCommitDiskFile();
    _commit_entries.reset(CCommitEntriesDisk(file));
}

CRaftMediator::~CRaftMediator() {

}

uint32_t CRaftMediator::GetId() {
    return _id;
}

void CRaftMediator::ConnectedNode(absl::string_view ip, uint16_t port) {
    std::shared_ptr<CNode> node(std::make_shared<NodeImpl>(_net));
    _node_map[absl::StrFormat("%s_%d", ip.data(), port)] = node;
}

void CRaftMediator::DisConnectedNode(absl::string_view ip, uint16_t port) {
    std::string net_handle = absl::StrFormat("%s_%d", ip.data(), port);
    auto iter = _node_map.find(net_handle);
    if (iter != _node_map.end()) {
        _node_map.erase(iter);
    }
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
}

void CRaftMediator::SendVoteToAll(VoteRequest& request) {
    for (auto iter = _node_map.begin(); iter != _node_map.end(); ++iter) {
        iter->second->SendVoteRequest(request);
    }
}

void CRaftMediator::SendHeartBeatToAll(HeartBeatResquest& request) {
    for (auto iter = _node_map.begin(); iter != _node_map.end(); ++iter) {
        iter->second->SendHeartRequest(request);
    }
}

void CRaftMediator::RecvHeartBeat(std::shared_ptr<CNode>& node, HeartBeatResquest& request) {
    _current_role->RecvHeartBeatRequest(node, request);
}

void CRaftMediator::SetCommitCallBack(absl::FunctionRef<void(std::string)> func) {
    _commit_call_back = func;
}