#include "Log.h"
#include "INet.h"
#include "Node.h"
#include "IRole.h"
#include "NodeManager.h"

using namespace raft;

CNodeManagerImpl::CNodeManagerImpl(std::shared_ptr<CNet>& net) : _net(net) {
    _net->SetNewConnectCallBack(std::bind(&CNodeManagerImpl::NewConnectCallBack, this, std::placeholders::_1));
    _net->SetDisConnectCallBack(std::bind(&CNodeManagerImpl::DisConnectCallBack, this, std::placeholders::_1));
    _net->SetHeartRequestRecvCallBack(std::bind(&CNodeManagerImpl::HeartRequestRecvCallBack, this, std::placeholders::_1, std::placeholders::_2));
    _net->SetHeartResponseRecvCallBack(std::bind(&CNodeManagerImpl::HeartResponseRecvCallBack, this, std::placeholders::_1, std::placeholders::_2));
    _net->SetVoteRequestRecvCallBack(std::bind(&CNodeManagerImpl::VoteRequestRecvCallBack, this, std::placeholders::_1, std::placeholders::_2));
    _net->SetVoteResponseRecvCallBack(std::bind(&CNodeManagerImpl::VoteResponseRecvCallBack, this, std::placeholders::_1, std::placeholders::_2));
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

void CNodeManagerImpl::ConnectTo(const std::string& ip, uint16_t port) {
    _net->ConnectTo(ip, port);
}

void CNodeManagerImpl::NewConnectCallBack(const std::string& net_handle) {
    std::shared_ptr<CNode> node(std::make_shared<NodeImpl>(_net, net_handle));
    _node_map[net_handle] = node;
    base::LOG_DEBUG("recv a new connection. handle : %s", net_handle.c_str());
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