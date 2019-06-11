#include <brpc/channel.h>
#include <memory>
#include "Log.h"
#include "CClientSDK.h"
#include "Tool.h"
#include "raft_rpc.pb.h"
#include <iostream>
using namespace raft;

struct CallBackBag {
    CClient* _cur;
    ::raft::ClientResponse* _response;
    std::string _msg;
    
    CallBackBag(CClient* cur, ::raft::ClientResponse* response, std::string msg) :
                     _cur(cur), _response(response), _msg(msg){
    }
};

CClient::CClient() : _ser_stub(nullptr){

}

CClient::~CClient() {

}

bool CClient::Init(const std::string& config_file) {
    _config_path = config_file;
    _config.SetFilePath(config_file);
    _config.ReLoadFile();

    // get node ip list from config
    std::string node_list = _config.GetStringValue("node_list");
    if (node_list.empty()) {
        LOG_ERROR("get node info list failed from config.");
        return false;
    }

    if (_ser_stub) {
        delete _ser_stub->channel();
        delete _ser_stub;
        _ser_stub = nullptr;
    }

    std::vector<std::string> node_info_list = SplitStr(node_list, ";");

    brpc::Channel *channel = new brpc::Channel;
    brpc::ChannelOptions options;
    options.succeed_without_server = true;
    std::string connect_node_ip;
    for (auto iter = node_info_list.begin(); iter != node_info_list.end(); ++iter) {
         channel->Init(iter->c_str(), &options);
         raft::RaftService_Stub stud(channel);
         if (!SayHello(&stud)) {
             LOG_ERROR("connect a new node failed. ip : %s", iter->c_str());

         } else {
             // only connect one 
             connect_node_ip = std::move(*iter);
             break;
         }
    }

    if (connect_node_ip.empty()) {
        LOG_ERROR("connect raft cluster failed.");
        return false;
    }

    _ser_stub = new raft::RaftService_Stub(channel);
    return true;
}

void CClient::SetCallBackFunc(const ClientCallBack& call_back) {
    _client_call_back = call_back;
}

bool CClient::SendMsg(const std::string& msg) {
    if (!_ser_stub) {
        return false;
    }
    ::raft::ClientRequest request;
    request.set_msg(msg);
    ::raft::ClientResponse* response = new ::raft::ClientResponse;
    brpc::Controller* cntl = new brpc::Controller;
    CallBackBag *param = new CallBackBag(this, response, msg);
    cntl->set_timeout_ms(10000);
    auto closure = google::protobuf::NewCallback(&raft::CClient::RpcDone, (void*)param, cntl);

    _ser_stub->client_msg(cntl, &request, response, closure);
}

void CClient::RpcDone(void* param, brpc::Controller* cntl) {
    auto cur = ((CallBackBag*)param)->_cur;
    auto response = ((CallBackBag*)param)->_response;
    auto msg = std::move(((CallBackBag*)param)->_msg);
    delete ((CallBackBag*)param);
    std::unique_ptr<raft::ClientResponse> response_guard(response);
    std::unique_ptr<brpc::Controller> cntl_guard(cntl);
    if (cntl->Failed()) {
        LOG_ERROR("rpc call server failed. err info : %s", cntl->ErrorText().c_str());
        if (cur->Init(cur->_config_path)){
            cur->SendMsg(msg);
        }
        return;
    }
    
    int err_code = response_guard->err_code();
    std::string err_des = response_guard->des();
    if (err_code == raft::ERR_Success) {
        cur->_client_call_back(err_code, err_des);

    } else if (err_code == raft::ERR_LessNodeRecv) {
        cur->_client_call_back(err_code, err_des);

    } else if (err_code == raft::ERR_NotLeader) {
        std::string leader_ip = response_guard->leader_ip();

        // connect leader
        brpc::Channel *channel = new brpc::Channel;
        brpc::ChannelOptions options;
        std::string connect_node_ip;
        if (channel->Init(leader_ip.c_str(), &options)) {
            LOG_ERROR("connect a new node failed. ip : %s", leader_ip.c_str());
            return;
        }

        delete cur->_ser_stub->channel();
        delete cur->_ser_stub;
        cur->_ser_stub = new raft::RaftService_Stub(channel);
        
        // send msg again
        cur->SendMsg(msg);
    }
}

bool CClient::SayHello(RaftService_Stub* stub) {
    raft::HelloResquest request;
    raft::HelloResponse response;
    brpc::Controller cntl;
    stub->rpc_hello(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        return true;

    } else {
        LOG_INFO("connect a new node failed.");
        return false;
    }
}