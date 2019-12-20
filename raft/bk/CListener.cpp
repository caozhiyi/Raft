#include <brpc/server.h>
#include "Log.h"
#include "CNode.h"
#include "raft_rpc.pb.h"
#include "CListener.h"

using namespace raft;

CListener::CListener(CNode* node) : _node(node) {

}

CListener::~CListener() {

}

void CListener::HandleClient(const std::string& ip_port, const std::string& msg, ::raft::ClientResponse* response,
    ::google::protobuf::Closure* done) {
    //judge current node is leader
    response->set_msg(msg);
    if (!_node->IsLeader()) {
        response->set_err_code(ERR_NotLeader);
        response->set_des("not a leader");
        response->set_leader_ip(_node->GetLeaderInfo());
        done->Run();
        return;
    }
    
    //put the msg to cur node server
    Time now_time = _node->HandleClientMsg(ip_port, msg);
    std::unique_lock<std::mutex> lock(_client_mutex);
    _client_msg[now_time] = std::make_pair((void*)response, (void*)done);
}

bool CListener::SendRet(Time& time_pos, int err_code, const std::string& des) {
    std::unique_lock<std::mutex> lock(_client_mutex);
    auto iter = _client_msg.find(time_pos);
    if (iter == _client_msg.end()) {
        return false;
    }

    auto response = (::raft::ClientResponse*)iter->second.first;
    auto done     = (::google::protobuf::Closure*)iter->second.second;
    
    response->set_err_code(err_code);
    response->set_des(des);
    done->Run();
 
    _client_msg.erase(iter);
}

void CListener::CleanMsg() {
    std::string leader_ip = _node->GetLeaderInfo();
    std::unique_lock<std::mutex> lock(_client_mutex);
    for (auto iter = _client_msg.begin(); iter != _client_msg.end(); ++iter) {
        auto response = (::raft::ClientResponse*)iter->second.first;
        auto done = (::google::protobuf::Closure*)iter->second.second;
    
        response->set_err_code(ERR_NotLeader);
        response->set_des("not a leader");
        response->set_leader_ip(_node->GetLeaderInfo());
        done->Run();
    }
    _client_msg.clear();
}