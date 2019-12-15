#include "NodeImpl.h"
#include "INet.h"

using namespace raft;

NodeImpl::NodeImpl(std::shared_ptr<CNet>& net) {
    _net = net;
}

NodeImpl::~NodeImpl() {

}

void NodeImpl::SetNetHandle(absl::string_view handle) {
    _net_handle = handle.data();
}

std::string NodeImpl::GetNetHandle() {
    return _net_handle;
}

uint64_t NodeImpl::GetNextIndex() {
    return _should_send_index;
}

void NodeImpl::SetNextIndex(uint64_t index) {
    _should_send_index = index;
}
 
uint64_t NodeImpl::GetNewestIndex() {
    return _sended_newest_index;
}

void NodeImpl::SetNewestIndex(uint64_t index) {
    _sended_newest_index = index;
}

void NodeImpl::SendHeartRequest(HeartBeatResquest& request) {
    _net->SendHeartRequest(_net_handle, request);
}

void NodeImpl::SendHeartResponse(HeartBeatResponse& response) {
    _net->SendHeartResponse(_net_handle, response);
}

void NodeImpl::SendVoteRequest(VoteRequest& request) {
    _net->SendVoteRequest(_net_handle, request);
}

void NodeImpl::SendVoteResponse(VoteResponse& response) {
    _net->SendVoteResponse(_net_handle, response);
}