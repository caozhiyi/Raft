#include "Log.h"
#include "Node.h"
#include "IConfig.h"
#include "RaftTimer.h"
#include "CppnetImpl.h"
#include "LeaderRole.h"
#include "ConfigImpl.h"
#include "NodeManager.h"
#include "RaftMediator.h"
#include "FollowerRole.h"
#include "CandidateRole.h"
#include "ClientManager.h"
#include "CommitEntriesDisk.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "absl/strings/str_format.h"

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

void CRaftMediator::Start(const std::string& config_file) {
    _config.reset(new CConfigImpl(config_file));

    // print log?
    bool print_log = _config->PrintLog();
    if (print_log) {
        std::string log_file = _config->GetLogFile();
        uint16_t log_level = _config->GetLogLevel();
        base::CLog::Instance().SetLogLevel((base::LogLevel)log_level);
        base::CLog::Instance().SetLogName(log_file);
        base::CLog::Instance().Start();
    }
    
    // start net io
    std::string ip = _config->GetIp();
    uint16_t port = _config->GetPort();
    uint16_t thread_num = _config->GetThreadNum();
    _net->Start(ip, port, thread_num);

    // connect to other node
    std::string node_address = _config->GetNodeInfo();
    _node_manager->ConnectToAll(node_address);
}

void CRaftMediator::Join() {
    _net->Join();
}

void CRaftMediator::Dealloc() {
    _net->Dealloc();
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
    _node_manager->SetRole(_current_role);
    _client_manager->SetRole(_current_role);
    if (type == follower_role) {
        _current_role = _follower_role;

    } else if (type == candidate_role) {
        _current_role = _candidate_role;

    } else if (type == leader_role) {
        _current_role = _leader_role;
        // send header beat to all
        _current_role->HeartBeatTimerOut();

    } else {
        base::LOG_ERROR("unknow role type.");
    }
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

const std::map<std::string, std::shared_ptr<CNode>>& CRaftMediator::GetAllNode() {
    return _node_manager->GetAllNode();
}

uint32_t CRaftMediator::GetNodeCount() {
    return _node_manager->GetNodeCount();
}