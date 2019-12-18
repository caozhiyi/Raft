#include "LeaderRole.h"
#include "INode.h"
#include "RaftMediator.h"

using namespace raft;

CLeaderRole::CLeaderRole(std::shared_ptr<CRoleData>& role_data, std::shared_ptr<CTimer>& timer, CRaftMediator* mediator) : CRole(role_data, timer, mediator) {

}

CLeaderRole::~CLeaderRole() {

}

ROLE_TYPE CLeaderRole::GetRole() {
    return leader_role;
}

void CLeaderRole::RecvVoteRequest(std::shared_ptr<CNode>& node, VoteRequest& vote_request) {
    VoteResponse response;
    response.set_term(_role_data->_current_term);
    response.set_vote_granted(true);
    if (vote_request.last_term() < _role_data->_current_term) {
        response.set_vote_granted(false);
    }
    // compare prev log index and term
    auto last_entries = _role_data->_entries_map.end()--;
    Entries& prev_entries = last_entries->second;
    if (prev_entries._index > vote_request.last_index() ||
        prev_entries._term > vote_request.last_term()) {
        response.set_vote_granted(false);
    }
    
    node->SendVoteResponse(response);
    // change role to follower
    _role_data->_role_change_call_back(follower_role);
}

void CLeaderRole::RecvHeartBeatRequest(std::shared_ptr<CNode>& node, HeartBeatResquest& heart_request) {
    HeartBeatResponse response;
    response.set_success(true);
    if (heart_request.term() < _role_data->_current_term) {
        response.set_success(false);
    }
    // compare prev log index and term
    auto last_entries = _role_data->_entries_map.end()--;
    Entries& prev_entries = last_entries->second;
    if (prev_entries._index < heart_request.prev_log_term() || 
        prev_entries._term < heart_request.prev_log_term()) {
        response.set_success(false);
    }
    if (response.success()) {
        // change role to follower
        _role_data->_role_change_call_back(follower_role);
    }
    // change to follower recv this request again
    _role_data->_raft_mediator->RecvHeartBeat(node, heart_request);
}

void CLeaderRole::RecvVoteResponse(std::shared_ptr<CNode>& node, VoteResponse& vote_response) {
    // do nothing
}

void CLeaderRole::RecvHeartBeatResponse(std::shared_ptr<CNode>& node, HeartBeatResponse& heart_response) {
    // send response to client
    // TODO
}

void CLeaderRole::RecvClientRequest(std::shared_ptr<CClient>& client, ClientRequest& request) {
    // get entries from client
    // TODO
}

void CLeaderRole::CandidateTimeOut() {
    // do nothing
}

void CLeaderRole::HeartBeatTimerOut() {
    // send heart request to all node again
    // TODO
}