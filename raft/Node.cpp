#include "Node.h"
#include "INet.h"

using namespace raft;

NodeImpl::NodeImpl(std::shared_ptr<CNet>& net, const std::string& handle) : _net(net),
                                                                            _net_handle(handle),
                                                                            _match_index(0),
                                                                            _should_send_index(0) {
}

NodeImpl::~NodeImpl() {

}

void NodeImpl::SetNetHandle(const std::string& handle) {
    _net_handle = handle.data();
}

const std::string& NodeImpl::GetNetHandle() {
    return _net_handle;
}

uint64_t NodeImpl::GetNextIndex() {
    return _should_send_index;
}

void NodeImpl::SetNextIndex(uint64_t index) {
    _should_send_index = index;
}
 
uint64_t NodeImpl::GetMatchIndex() {
    return _match_index;
}

void NodeImpl::SetMatchIndex(uint64_t index) {
    _match_index = index;
}

void NodeImpl::SendNodeInfoRequest(NodeInfoRequest& request) {
    _net->SendNodeInfoRequest(_net_handle, request);
}

void NodeImpl::SendNodeInfoResponse(NodeInfoResponse& response) {
    _net->SendNodeInfoResponse(_net_handle, response);
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

void NodeImpl::SendEntriesRequest(EntriesRequest& request) {
    _net->SendEntriesRequest(_net_handle, request);
}

void NodeImpl::SendEntriesResponse(EntriesResponse& response) {
    _net->SendEntriesResponse(_net_handle, response);
}