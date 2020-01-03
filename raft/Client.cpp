#include "Log.h"
#include "INet.h"
#include "Client.h"

using namespace raft;

CClientImpl::CClientImpl(std::shared_ptr<CNet>& net, const std::string& handle) : _net(net), _net_handle(handle) {

}

CClientImpl::~CClientImpl() {

}

void CClientImpl::SetNetHandle(const std::string& handle) {
    _net_handle = handle;
}

std::string CClientImpl::GetNetHandle() {
    return _net_handle;
}

void CClientImpl::SendToClient(ClientResponse& response) {
    base::LOG_DEBUG("sned response to %s, context : %s. handle : %s", _net_handle.c_str(), response.DebugString().c_str());
    _net->SendClientResponse(_net_handle, response);
}