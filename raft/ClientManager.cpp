#include <functional>

#include "Log.h"
#include "INet.h"
#include "IRole.h"
#include "Client.h"
#include "ClientManager.h"

using namespace raft;

CClientManagerImpl::CClientManagerImpl(std::shared_ptr<CNet>& net) : _net(net){
    _net->SetClientRecvCallBack(std::bind(&CClientManagerImpl::RecvClientRequest, this, std::placeholders::_1, std::placeholders::_2));
    _net->SetClientConnectCallBack(std::bind(&CClientManagerImpl::ClientConnect, this, std::placeholders::_1));
    _net->SetClientDisConnectCallBack(std::bind(&CClientManagerImpl::ClientDisConnect, this, std::placeholders::_1));
}

CClientManagerImpl::~CClientManagerImpl() {

}

void CClientManagerImpl::SetRole(std::shared_ptr<CRole>& role) {
    _current_role = role;
}

void CClientManagerImpl::SendToAll(ClientResponse& response) {
    for (auto iter = _client_map.begin(); iter != _client_map.end(); ++iter) {
        iter->second->SendToClient(response);
    }
}

uint32_t CClientManagerImpl::GetNodeCount() {
    return (uint32_t)_client_map.size();
}

void CClientManagerImpl::RecvClientRequest(const std::string& net_handle, ClientRequest& request) {
    std::shared_ptr<CClient> client;
    auto iter = _client_map.find(net_handle);
    // create a client
    if (iter == _client_map.end()) {
        client.reset(new CClientImpl(_net, net_handle));
        _client_map[net_handle] = client;
        base::LOG_DEBUG("get a new client connection. handle : %s", net_handle.c_str());

    } else {
        client = iter->second;
    }
    _current_role->RecvClientRequest(client, request);
}

void CClientManagerImpl::ClientConnect(const std::string& net_handle) {
    // create a client
    std::shared_ptr<CClient> client(new CClientImpl(_net, net_handle));
    _client_map[net_handle] = client;
    base::LOG_DEBUG("get a new client connection. handle : %s", net_handle.c_str());
}

void CClientManagerImpl::ClientDisConnect(const std::string& net_handle) {
    auto iter = _client_map.find(net_handle);
    if (iter != _client_map.end()) {
        _client_map.erase(iter);
    }
    base::LOG_DEBUG("we lost a client connection. handle : %s", net_handle.c_str());
}