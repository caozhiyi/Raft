#include "Log.h"
#include "CClient.h"
#include "Tool.h"
#include "client_rpc.pb.h"

#include <iostream>

CClient::CClient() : _channel(nullptr){

}

CClient::~CClient() {

}

bool CClient::Init(const std::string& config_file) {
    _config.SetFilePath(config_file);
    _config.ReLoadFile();

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

    std::vector<std::string> node_info_list = SplitStr(node_list);

    baidu::rpc::Channel *channel = new  baidu::rpc::Channel;
    baidu::rpc::ChannelOptions options;
    std::string connect_node_ip;
    for (auto iter = node_info_list.begin(); iter != node_info_list.end(); ++iter) {
        if (channel.Init(iter->c_str(), &options) != 0) {
            LOG_ERROR("connect a new node failed. ip : %s", iter->c_str());

        } else {
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