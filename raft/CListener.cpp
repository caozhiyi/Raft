#include "Log.h"
#include "CNode.h"
#include "CClientRpc.h"

#include <iostream>

CListener::CListener() {

}

CListener::~CListener() {

}

bool CListener::Init(const std::string& ip_port) {
    _ip_port = ip_port;
    
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
    
}

bool CListener::SendRet(const std::string& ip_port, int err_code, const std::string& des) {

}