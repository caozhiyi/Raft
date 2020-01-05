#include "Log.h"
#include "RoleData.h"
#include "RaftTimer.h"
#include "message.pb.h"
#include "CppnetImpl.h"
#include "LeaderRole.h"
#include "ConfigImpl.h"
#include "NodeManager.h"
#include "MountClient.h"
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
    // create net 
    _net.reset(new CCppNet());

    // creata common data
    _common_data.reset(new CRoleData());

    _common_data->_timer.reset(new CTimerImpl());
    _common_data->_timer->SetHeartCallBack(std::bind(&CRaftMediator::HeartBeatTimerOut, this));
    _common_data->_timer->SetVoteCallBack(std::bind(&CRaftMediator::CandidateTimeOut, this));
    _common_data->_timer->Start();

    _common_data->_recv_heart_again = std::bind(&CRaftMediator::RecvHeartBeat, this, std::placeholders::_1, std::placeholders::_2);
    _common_data->_role_change_call_back = std::bind(&CRaftMediator::ChangeRole, this, std::placeholders::_1, std::placeholders::_2);
    _common_data->_commit_entries_call_back = std::bind(&CRaftMediator::CommitEntries, this, std::placeholders::_1);

    // create all role control 
    _leader_role.reset(new CLeaderRole(_common_data));
    _candidate_role.reset(new CCandidateRole(_common_data));
    _follower_role.reset(new CFollowerRole(_common_data));


    // create client manager
    _client_manager.reset(new CClientManagerImpl(_net));

    // create mount client 
    _mount_client.reset(new CMountClient(_net));
}

CRaftMediator::~CRaftMediator() {

}

void CRaftMediator::Start(const std::string& config_file) {
    _config.reset(new CConfigImpl(config_file));

    base::LOG_DEBUG("start with config : %s", config_file.c_str());
    // create commit entries
    std::string file = _config->GetCommitDiskFile();
    _commit_entries.reset(new CCommitEntriesDisk(file));

    _common_data->_cur_node_id = _config->GetNodeId();
    _common_data->_heart_time = _config->GetHeartTime();
    _common_data->_candidate_time = _config->GetVoteTimerRandomRange();

    uint16_t thread_num = _config->GetThreadNum();
    base::LOG_DEBUG("cur_node_id : %d, heart_time : %d, candidate_time: [%d,%d], thread_num: %d", _common_data->_cur_node_id,
            _common_data->_heart_time, _common_data->_candidate_time.first, _common_data->_candidate_time.second, thread_num);
    _net->Init(thread_num);

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
    
    _common_data->_cur_net_handle = absl::StrFormat("%s:%d", ip.c_str(), port);
    _common_data->_node_manager.reset(new CNodeManagerImpl(_net, _common_data->_cur_net_handle));
    _net->Start(ip, port);

    // connect to other node
    std::string node_address = _config->GetNodeInfo();
    _common_data->_node_manager->ConnectToAll(node_address);

    // set current role to follower
    ChangeRole(follower_role, "");
}

void CRaftMediator::Join() {
    _net->Join();
}

void CRaftMediator::Dealloc() {
    _net->Dealloc();
}

void CRaftMediator::CommitEntries(Entries& entries) {
    if (_commit_call_back) {
        _commit_call_back(entries._entries);
    }
    
    _commit_entries->PushEntries(entries);
}

void CRaftMediator::ChangeRole(ROLE_TYPE type, const std::string& net_handle) {
    base::LOG_DEBUG("change role to %d", type);
    _mount_client->SetLeaderHandle(net_handle);
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
    _common_data->_node_manager->SetRole(_current_role);
    _client_manager->SetRole(_current_role);
    _mount_client->SetCurRole(_current_role);
    _current_role->ItsMyTurn();
}

void CRaftMediator::PushEntries(const std::string& entries) {
    _mount_client->SendEntries(entries);
}

void CRaftMediator::RecvHeartBeat(std::shared_ptr<CNode>& node, HeartBeatResquest& request) {
    _current_role->RecvHeartBeatRequest(node, request);
}

void CRaftMediator::SetCommitCallBack(const std::function<void(std::string)>& func) {
    _commit_call_back = func;
}

void CRaftMediator::CandidateTimeOut() {
    _current_role->CandidateTimeOut();
}

void CRaftMediator::HeartBeatTimerOut() {
    _current_role->HeartBeatTimerOut();
}