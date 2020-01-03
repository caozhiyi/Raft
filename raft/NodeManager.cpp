#include "Log.h"
#include "INet.h"
#include "Node.h"
#include "IRole.h"
#include "NodeManager.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"

using namespace raft;

CNodeManagerImpl::CNodeManagerImpl(std::shared_ptr<CNet>& net) : _net(net) {
    _net->SetNewConnectCallBack(std::bind(&CNodeManagerImpl::NewConnectCallBack, this, std::placeholders::_1));
    _net->SetDisConnectCallBack(std::bind(&CNodeManagerImpl::DisConnectCallBack, this, std::placeholders::_1));
    _net->SetHeartRequestRecvCallBack(std::bind(&CNodeManagerImpl::HeartRequestRecvCallBack, this, std::placeholders::_1, std::placeholders::_2));
    _net->SetHeartResponseRecvCallBack(std::bind(&CNodeManagerImpl::HeartResponseRecvCallBack, this, std::placeholders::_1, std::placeholders::_2));
    _net->SetVoteRequestRecvCallBack(std::bind(&CNodeManagerImpl::VoteRequestRecvCallBack, this, std::placeholders::_1, std::placeholders::_2));
    _net->SetVoteResponseRecvCallBack(std::bind(&CNodeManagerImpl::VoteResponseRecvCallBack, this, std::placeholders::_1, std::placeholders::_2));
    _net->SetNodeInfoRequestCallBack(std::bind(&CNodeManagerImpl::NodeInfoRequestCallBack, this, std::placeholders::_1, std::placeholders::_2));
    _net->SetNodeInfoResponseCallBack(std::bind(&CNodeManagerImpl::NodeInfoResponseCallBack, this, std::placeholders::_1, std::placeholders::_2));
}

CNodeManagerImpl::~CNodeManagerImpl() {

}

void CNodeManagerImpl::SetRole(std::shared_ptr<CRole>& role) {
    _current_role = role;
}

void CNodeManagerImpl::SendHeartToAll(HeartBeatResquest& request) {
    for (auto iter = _node_map.begin(); iter != _node_map.end(); ++iter) {
        iter->second->SendHeartRequest(request);
    }
}

void CNodeManagerImpl::SendVoteToAll(VoteRequest& request) {
    for (auto iter = _node_map.begin(); iter != _node_map.end(); ++iter) {
        iter->second->SendVoteRequest(request);
    }
}

uint32_t CNodeManagerImpl::GetNodeCount() {
    return (uint32_t)_node_map.size();
}

const std::map<std::string, std::shared_ptr<CNode>>& CNodeManagerImpl::GetAllNode() {
    return _node_map;
}

void CNodeManagerImpl::ConnectToAll(const std::string& net_handle_list) {
    std::vector<std::string> addr_vec = absl::StrSplit(net_handle_list, ";");
    for (size_t i = 0; i < addr_vec.size(); i++) {
        if (addr_vec[i] .empty() || _node_map.count(addr_vec[i]) > 0) {
            continue;
        }

        base::LOG_ERROR("try to connect to %s", addr_vec[i].c_str());
        std::vector<std::string> handle_vec = absl::StrSplit(addr_vec[i], ":");
        if (handle_vec.size() == 2) {
            uint32_t port = 0;
            if (!absl::SimpleAtoi<uint32_t>(handle_vec[1], &port)) {
                base::LOG_ERROR("parser net handle failed.");
            }
            _net->ConnectTo(handle_vec[0], (uint16_t)port);
        }
    }
}

void CNodeManagerImpl::ConnectTo(const std::string& ip, uint16_t port) {
    _net->ConnectTo(ip, port);
}

void CNodeManagerImpl::NewConnectCallBack(const std::string& net_handle) {
    std::shared_ptr<CNode> node(std::make_shared<NodeImpl>(_net, net_handle));
    _node_map[net_handle] = node;
    base::LOG_DEBUG("recv a new connection. handle : %s", net_handle.c_str());

    // send all node info to new node
    NodeInfoRequest request;
    for (auto iter = _node_map.begin(); iter != _node_map.end(); ++iter) {
        request.add_net_handle(iter->first);
    }
    node->SendNodeInfoRequest(request);
}

void CNodeManagerImpl::DisConnectCallBack(const std::string& net_handle) {
    auto iter = _node_map.find(net_handle);
    if (iter != _node_map.end()) {
        _node_map.erase(iter);
    }
    base::LOG_DEBUG("we lost a connection. handle : %s", net_handle.c_str());
}

void CNodeManagerImpl::HeartRequestRecvCallBack(const std::string& net_handle, HeartBeatResquest& request) {
    std::shared_ptr<CNode> node = GetNode(net_handle);
    _current_role->RecvHeartBeatRequest(node, request);
}

void CNodeManagerImpl::HeartResponseRecvCallBack(const std::string& net_handle, HeartBeatResponse& response) {
    std::shared_ptr<CNode> node = GetNode(net_handle);
    _current_role->RecvHeartBeatResponse(node, response);
}

void CNodeManagerImpl::VoteRequestRecvCallBack(const std::string& net_handle, VoteRequest& request) {
    std::shared_ptr<CNode> node = GetNode(net_handle);
    _current_role->RecvVoteRequest(node, request);
}

void CNodeManagerImpl::VoteResponseRecvCallBack(const std::string& net_handle, VoteResponse& response) {
    std::shared_ptr<CNode> node = GetNode(net_handle);
    _current_role->RecvVoteResponse(node, response);
}

void CNodeManagerImpl::NodeInfoRequestCallBack(const std::string& net_handle, NodeInfoRequest& request) {
    base::LOG_DEBUG("get a node info request from %s, context: %s", net_handle.c_str(), request.DebugString().c_str());

    // connect all node from request
    auto size = request.net_handle_size();
    for (int i = 0; i < size; i++) {
        std::string net_handle = request.net_handle(i);
        if (_node_map.count(net_handle) == 0) {
            continue;
        }
        std::vector<std::string> handle_vec = absl::StrSplit(net_handle, ":");
        if (handle_vec.size() == 2) {
            uint32_t port = 0;
            if (!absl::SimpleAtoi<uint32_t>(handle_vec[1], &port)) {
                base::LOG_ERROR("parser net handle failed.");
            }
            _net->ConnectTo(handle_vec[0], (uint16_t)port);
        }
    }
    
    // send response
    NodeInfoResponse response;
    for (auto iter = _node_map.begin(); iter != _node_map.end(); ++iter) {
        response.add_net_handle(iter->first);
    }
    auto node = GetNode(net_handle);
    node->SendNodeInfoResponse(response);
}

void CNodeManagerImpl::NodeInfoResponseCallBack(const std::string& net_handle, NodeInfoResponse& response) {
    base::LOG_DEBUG("get a node info response from %s, context: %s", net_handle.c_str(), response.DebugString().c_str());
    // connect all node from response
    auto size = response.net_handle_size();
    for (int i = 0; i < size; i++) {
        std::string net_handle = response.net_handle(i);
        if (_node_map.count(net_handle) != 0) {
            continue;
        }
        std::vector<std::string> handle_vec = absl::StrSplit(net_handle, ":");
        if (handle_vec.size() == 2) {
            uint32_t port = 0;
            if (!absl::SimpleAtoi<uint32_t>(handle_vec[1], &port)) {
                base::LOG_ERROR("parser net handle failed.");
            }
            _net->ConnectTo(handle_vec[0], (uint16_t)port);
        }
    }
}

std::shared_ptr<CNode> CNodeManagerImpl::GetNode(const std::string& net_handle) {
    std::shared_ptr<CNode> node;
    auto iter = _node_map.find(net_handle);
    // create a node
    if (iter == _node_map.end()) {
        node.reset(new NodeImpl(_net, net_handle));
        _node_map[net_handle] = node;
        base::LOG_DEBUG("get a new node connection. handle : %s", net_handle.c_str());

    } else {
        node = iter->second;
    }
    return node;
}