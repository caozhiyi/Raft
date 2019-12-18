#include "Client.h"
#include "INet.h"

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
    _net->SendToClient(_net_handle, response);
}