#include <brpc/channel.h>
#include <memory>
#include "Log.h"
#include "CClient.h"
#include "Tool.h"
#include "client_rpc.pb.h"

using namespace raft;

CClient::CClient() : _channel(nullptr){

}

CClient::~CClient() {

}

bool CClient::Init(const std::string& config_file) {
    _config.SetFilePath(config_file);
    _config.ReLoadFile();

    // get node ip list from config
    std::string node_list = _config.GetStringValue("node_list");
    if (node_list.empty()) {
        LOG_ERROR("get node info list failed from config.");
        return false;
    }

    if (_channel) {
        delete _channel->channel();
        delete _channel;
        _channel = nullptr;
    }

    std::vector<std::string> node_info_list = SplitStr(node_list, ";");

    brpc::Channel *channel = new brpc::Channel;
    brpc::ChannelOptions options;
    std::string connect_node_ip;
    for (auto iter = node_info_list.begin(); iter != node_info_list.end(); ++iter) {
        if (channel->Init(iter->c_str(), &options) != 0) {
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

    _channel = new raft::ClientService_Stub(channel);
    return true;
}

void CClient::SetCallBackFunc(const ClientCallBack& call_back) {
    _client_call_back = call_back;
}

bool CClient::SendMsg(const std::string& msg) {
    if (!_channel) {
        return false;
    }
    ::raft::ClientRequest request;
    request.set_msg(msg);
    ::raft::ClientResponse* response = new ::raft::ClientResponse;
    brpc::Controller* cntl = new brpc::Controller;
    std::pair<CClient*, ::raft::ClientResponse*> *param = new std::pair<CClient*, ::raft::ClientResponse*>;
    param->first = this;
    param->second = response;
    cntl->set_timeout_ms(1000000);
    auto closure = google::protobuf::NewCallback(&raft::CClient::RpcDone, (void*)param, cntl);

    _channel->client_msg(cntl, &request, response, closure);
}

void CClient::RpcDone(void* param, brpc::Controller* cntl) {
    CClient* cur = ((std::pair<CClient*, ::raft::ClientResponse*>*)param)->first;
    raft::ClientResponse* response = ((std::pair<CClient*, ::raft::ClientResponse*>*)param)->second;
    delete (std::pair<CClient*, ::raft::ClientResponse*>*)param;

    std::unique_ptr<raft::ClientResponse> response_guard(response);
    std::unique_ptr<brpc::Controller> cntl_guard(cntl);
    if (cntl->Failed()) {
        LOG_ERROR("rpc call server failed. err info : %s", cntl->ErrorText().c_str());
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
        std::string msg = response_guard->msg();

        // connect leader
        brpc::Channel *channel = new brpc::Channel;
        brpc::ChannelOptions options;
        std::string connect_node_ip;
        if (channel->Init(leader_ip.c_str(), &options) != 0) {
            LOG_ERROR("connect a new node failed. ip : %s", leader_ip.c_str());
            return;
        }

        delete cur->_channel;
        cur->_channel = new raft::ClientService_Stub(channel);
        
        // send msg again
        cur->SendMsg(msg);
    }
}