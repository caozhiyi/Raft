#include "Log.h"
#include "CNode.h"
#include "client_rpc.pb.h"
#include "CListener.h"

#include <iostream>

CListener::CListener() {

}

CListener::~CListener() {

}

bool CListener::Init(const std::string& ip_port, CNode* node) {
    _ip_port = ip_port;
    _node = node;

    baidu::rpc::Server server;
    server.set_version("raft_server_1.0");
    raft::CClientRpc node(this);
    if (server.AddService(&node, baidu::rpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG_ERROR("fail to add service.");
        return false;
    }

    baidu::rpc::ServerOptions options;
    if (server.Start(_local_ip_port.c_str(), &options) != 0) {
        LOG_ERROR("Fail to start storage_server. ip : %s", _local_ip_port.c_str());
        return false;
    }

    server.RunUntilAskedToQuit();
    return true;
}

void CListener::HandleClient(const std::string& ip_port, const std::string& msg, ::raft_rpc::ClientResponse* response,
    ::google::protobuf::Closure* done) {
    //judge current node is leader
    if (!_node->IsLeader()) {
        response->set_err_code(1);
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

    auto response = (::raft_rpc::ClientResponse*)iter->second.first;
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
        auto response = (::raft_rpc::ClientResponse*)iter->second.first;
        auto done = (::google::protobuf::Closure*)iter->second.second;
    
        response->set_err_code(ERR_NotLeader);
        response->set_des("not a leader");
        response->set_leader_ip(_node->GetLeaderInfo());
        done->Run();
    }
    _client_msg.clear();
}