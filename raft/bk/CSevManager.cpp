#include <brpc/server.h>
#include "Log.h"
#include "CNode.h"
#include "raft_rpc.pb.h"
#include "CSevManager.h"
#include "UnixOs.h"
#include "Tool.h"
#include "config.h"
#include "CNodeRpc.h"
#include "CMsgRouter.h"

using namespace raft;

CSevManager::CSevManager() {

}

CSevManager::~CSevManager() {

}

bool CSevManager::Init(CConfig& config, CMsgRouter* router) {
    // init net. begin listen
    std::string ip = GetOsIp();
    if (ip.empty()) {
        LOG_ERROR("get local ip failed.");
        return false;
    }
    _local_ip_port = ip + ":" + config.GetStringValue("server");

    brpc::Server server;
    server.set_version(rpc_server_version);
    // set roter
    raft::CNodeRpc node(router);
    if (server.AddService(&node, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG_ERROR("fail to add service.");
        return false;
    }

    brpc::ServerOptions options;
    if (server.Start(_local_ip_port.c_str(), &options) != 0) {
        LOG_ERROR("Fail to start server. ip : %s", _local_ip_port.c_str());
        return false;
    }

    // is first node?
    bool is_first_node = config.GetIntValue("is_first_node");

    if (!is_first_node) {
        // add a new node to cluster
        std::vector<std::string> node_info_list;
        std::string node_list = config.GetStringValue("node_list");

        node_info_list = SplitStr(node_list, ";");

        brpc::Channel *channel = new  brpc::Channel;
        brpc::ChannelOptions options;
        std::string connect_node_ip;

        for (auto iter = node_info_list.begin(); iter != node_info_list.end(); ++iter) {
            if (*iter == _local_ip_port) {
                continue;
            }
            LOG_INFO("try to connect node. ip : %s", iter->c_str());
            channel->Init(iter->c_str(), &options);
            raft::RaftService_Stub stub(channel);
            // test if we can connect the node
            if (SayHello(&stub)) {
                connect_node_ip = std::move(*iter);
                break;

            }
            else {
                LOG_ERROR("connect a new node failed. ip : %s", iter->c_str());
            }
        }

        if (connect_node_ip.empty()) {
            LOG_ERROR("connect a new node failed. but current node is't the firstly.");
            return false;
        }
        LOG_INFO("connect to one node. ip : %s", connect_node_ip.c_str());

        raft::RaftService_Stub *stub = new raft::RaftService_Stub(channel);
        _channel_map[connect_node_ip] = stub;

        // get all node info
        raft::NodeInfoRequest request;
        raft::NodeInfoResponse response;
        request.set_local_ip(_local_ip_port);
        brpc::Controller cntl;
        cntl.set_timeout_ms(rpc_time_out);
        stub->rpc_node_info(&cntl, &request, &response, NULL);
        if (cntl.Failed()) {
            LOG_ERROR("rpc_node_info called failed. err:", cntl.ErrorText().c_str());
        }

        std::vector<std::string> info_vec;
        for (int i = 0; i < response.ip_port_size(); i++) {
            LOG_INFO("get node ip from other node. ip : %s", response.ip_port(i).c_str());
            info_vec.push_back(response.ip_port(i));
        }
        // connect all node
        AddNewNode(connect_node_ip, info_vec);

        BroadCastNodeInfo();
    }

    server.RunUntilAskedToQuit();

    return true;
}

bool CSevManager::SendSyncHeart(std::string ip_info, Time log_version, std::vector<std::string> msg_vec) {
    raft::HeartRequest request;
    raft::HeartResponse response;
    request.set_local_ip(_local_ip_port);
    request.set_done_msg(false);
    request.set_version(log_version);

    for (auto iter = msg_vec.begin(); iter != msg_vec.end(); ++iter) {
        request.add_msg(*iter);
    }

    if (request.msg_size() == 0) {
        return false;
    }

    auto stub = _channel_map.find(ip_info);
    if (stub->second) {
        brpc::Controller cntl;
        cntl.set_timeout_ms(rpc_time_out);
        stub->second->rpc_heart(&cntl, &request, &response, NULL);
        if (cntl.Failed()) {
            LOG_ERROR("rpc_heart called failed. err:", cntl.ErrorText().c_str());
            return false;
        }
    }
    return true;
}

bool CSevManager::SendAllHeart(Time log_version, Time new_version, std::vector<std::string> msg_vec,
    bool done_msg, std::vector<std::pair<std::string, Time>>& sync_vec) {

    raft::HeartRequest request;
    request.set_local_ip(_local_ip_port);

    request.set_version(log_version);
    for (auto iter = msg_vec.begin(); iter != msg_vec.end(); ++iter) {
        request.add_msg(*iter);
    }
    request.set_done_msg(done_msg);

    bool have_data = msg_vec.size() > 0;
    int back_count = 1;
    std::unique_lock<std::mutex> lock(_stub_mutex);
    for (auto iter = _channel_map.begin(); iter != _channel_map.end(); ++iter) {
        raft::HeartResponse response;
        brpc::Controller cntl;
        cntl.set_timeout_ms(rpc_time_out);
        iter->second->rpc_heart(&cntl, &request, &response, NULL);
        if (cntl.Failed()) {
            LOG_ERROR("rpc_heart called failed. err:", cntl.ErrorText().c_str());
            delete iter->second->channel();
            delete iter->second;
            iter = _channel_map.erase(iter);
            if (iter == _channel_map.end()) {
                break;
            }
            continue;
        }
        // new sync
        if (response.version() != new_version) {
            LOG_INFO("a node sync from me. ip:%s", iter->first.c_str());
            sync_vec.push_back(std::make_pair(iter->first, response.version()));

        } else {
            back_count++;
        }
    }

    if (back_count >= _channel_map.size() / 2 && have_data) {
        return true;

    } else {
        return false;
    }
}

bool CSevManager::SendAllVote(Time log_versoin) {
    ::raft::VoteResuest request;
    request.set_local_ip(_local_ip_port);
    request.set_version(log_versoin);

    int vote_count = 1;	// vote's num
    {
        std::unique_lock<std::mutex> lock(_stub_mutex);
        for (auto iter = _channel_map.begin(); iter != _channel_map.end(); ++iter) {
            ::raft::VoteResponse response;
            brpc::Controller cntl;
            cntl.set_timeout_ms(rpc_time_out);
            iter->second->rpc_vote(&cntl, &request, &response, NULL);
            if (cntl.Failed()) {
                LOG_ERROR("rpc_vote called failed. err:", cntl.ErrorText().c_str());
            }
            LOG_INFO("get a vote from ip:%s", iter->first.c_str());
            if (response.vote()) {
                vote_count++;
            }
        }
    }

    // to be a leader
    if (vote_count >= _channel_map.size() / 2 && vote_count > 1) {
        return true;
        LOG_INFO("I'm a leader now. ip:%s", _local_ip_port.c_str());
    }
    return false;
}

void CSevManager::BroadCastNodeInfo() {
    raft::NodeInfoRequest request;
    raft::NodeInfoResponse response;
    request.set_local_ip(_local_ip_port);

    // add local node info
    request.add_ip_port(_local_ip_port);
    std::unique_lock<std::mutex> lock(_stub_mutex);
    for (auto iter = _channel_map.begin(); iter != _channel_map.end(); ++iter) {
        request.add_ip_port(iter->first);
    }
    for (auto iter = _channel_map.begin(); iter != _channel_map.end(); ++iter) {
        brpc::Controller cntl;
        cntl.set_timeout_ms(rpc_time_out);
        iter->second->rpc_node_info(&cntl, &request, &response, NULL);
        if (cntl.Failed()) {
            LOG_ERROR("rpc_node_info called failed. err:", cntl.ErrorText().c_str());
        }
    }
}

void CSevManager::GetNodeInfo(const std::string& ip_port, std::vector<std::string>& node_info) {
    // if reconnect
    auto old_iter = _channel_map.find(ip_port);
    if (old_iter != _channel_map.end()) {
        delete old_iter->second;
        _channel_map.erase(old_iter);
    }

    // connect the new node
    brpc::Channel *channel = new  brpc::Channel;
    brpc::ChannelOptions options;
    if (channel->Init(ip_port.c_str(), &options) != 0) {
        LOG_ERROR("connect a new node failed. ip : %s", ip_port.c_str());
        return;
    }
    raft::RaftService_Stub *stub = new raft::RaftService_Stub(channel);
    std::unique_lock<std::mutex> lock(_stub_mutex);
    _channel_map[ip_port] = stub;

    // get all old node
    node_info.push_back(_local_ip_port);
    for (auto iter = _channel_map.begin(); iter != _channel_map.end(); ++iter) {
        node_info.push_back(iter->first);
    }
    return;
}

void CSevManager::AddNewNode(const std::string& ip_port, const std::vector<std::string>& node_info) {
    std::unique_lock<std::mutex> lock(_stub_mutex);
    // add all node info
    for (size_t i = 0; i < node_info.size(); i++) {
        if (node_info[i] == _local_ip_port) {
            continue;
        }
        auto old_iter = _channel_map.find(node_info[i]);
        if (old_iter != _channel_map.end()) {
            continue;
        }

        brpc::Channel *channel = new brpc::Channel;
        brpc::ChannelOptions options;
        if (channel->Init(node_info[i].c_str(), &options) != 0) {
            LOG_ERROR("connect a new node failed. ip : %s", node_info[i].c_str());
            continue;
        }
        LOG_INFO("connect to a new node. ip : %s", node_info[i].c_str());
        raft::RaftService_Stub *stub = new raft::RaftService_Stub(channel);
        _channel_map[node_info[i]] = stub;
    }
}

bool CSevManager::SayHello(RaftService_Stub* stub) {
    raft::HelloResquest request;
    raft::HelloResponse response;
    brpc::Controller cntl;
    cntl.set_timeout_ms(rpc_time_out);
    stub->rpc_hello(&cntl, &request, &response, NULL);
    if (!cntl.Failed()) {
        LOG_ERROR("connect a new node success.");
        return true;

    } else {
        LOG_ERROR("connect a new node failed.");
        return false;
    }
}