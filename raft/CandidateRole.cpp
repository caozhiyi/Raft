#include "Log.h"
#include "INode.h"
#include "ITimer.h"
#include "IClient.h"
#include "RoleData.h"
#include "INodeManager.h"
#include "CandidateRole.h"

using namespace raft;

CCandidateRole::CCandidateRole(std::shared_ptr<CRoleData>& role_data) : CRole(role_data) {

}

CCandidateRole::~CCandidateRole() {

}

ROLE_TYPE CCandidateRole::GetRole() {
    return candidate_role;
}

void CCandidateRole::ItsMyTurn() {
    // vote for myself
    _role_data->_vote_num = 1;
    // send all node to get vote
    VoteRequest request;
    request.set_term(_role_data->_current_term + 1);
    request.set_candidate_id(_role_data->_cur_node_id);
    request.set_last_term(_role_data->_current_term);
    request.set_last_index(_role_data->_newest_index);
    _role_data->_node_manager->SendVoteToAll(request);
    base::LOG_DEBUG("candidate send vote request to all node, %s", request.DebugString().c_str());

    // start candidate timer
    auto range = _role_data->_candidate_time;
    uint32_t time = absl::uniform_int_distribution<uint32_t>(range.first, range.second)(_role_data->_gen);
    _role_data->_timer->StartVoteTimer(time);
}

void CCandidateRole::RecvVoteRequest(std::shared_ptr<CNode>& node, VoteRequest& vote_request) {
    base::LOG_DEBUG("candidate recv vote request from node, %s, context : %s", 
                node->GetNetHandle().c_str(), vote_request.DebugString().c_str());

    VoteResponse response;
    response.set_term(_role_data->_current_term);
    response.set_vote_granted(true);
    // compare term
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

    // already vote to other node
    if (_role_data->_voted_for_id != 0 && _role_data->_voted_for_id == vote_request.candidate_id()) {
        response.set_vote_granted(false);
    }

    if (response.vote_granted()) {
        _role_data->_voted_for_id = vote_request.candidate_id();
    }

    node->SendVoteResponse(response);
}

void CCandidateRole::RecvHeartBeatRequest(std::shared_ptr<CNode>& node, HeartBeatResquest& heart_request) {
    base::LOG_DEBUG("candidate recv heart request from node, %s, context : %s", 
                node->GetNetHandle().c_str(), heart_request.DebugString().c_str());
    // set leader net handle
    if (_role_data->_leader_net_handle != node->GetNetHandle()) {
        _role_data->_leader_net_handle = node->GetNetHandle();
    }

    // change role to follower
    _role_data->_role_change_call_back(follower_role, node->GetNetHandle());
    // recv that request again
    _role_data->_recv_heart_again(node, heart_request);
}

void CCandidateRole::RecvVoteResponse(std::shared_ptr<CNode>& node, VoteResponse& vote_response) {
    base::LOG_DEBUG("candidate recv vote response from node, %s, context : %s", 
                node->GetNetHandle().c_str(), vote_response.DebugString().c_str());

    if (vote_response.vote_granted()) {
        _role_data->_vote_num++;
    }
    if (_role_data->_vote_num > _role_data->_node_manager->GetNodeCount() / 2) {
        _role_data->_vote_num = 0;
        // to be a leader
        _role_data->_role_change_call_back(leader_role, _role_data->_cur_net_handle);
        _role_data->_current_term++;
    }
}

void CCandidateRole::RecvHeartBeatResponse(std::shared_ptr<CNode>& node, HeartBeatResponse& heart_response) {
    // do nothing
    base::LOG_DEBUG("candidate recv heart response from node, %s, context : %s", 
                node->GetNetHandle().c_str(), heart_response.DebugString().c_str());
}

void CCandidateRole::RecvClientRequest(std::shared_ptr<CClient>& client, ClientRequest& request) {
    base::LOG_DEBUG("candidate recv client request from node, %s, context : %s", 
                client->GetNetHandle().c_str(), request.DebugString().c_str());

    // tell client resend again, i don't know who is leader.
    ClientResponse response;
    response.set_ret_code(send_again);
    client->SendToClient(response);
}

void CCandidateRole::CandidateTimeOut() {
    // reset candidate timer 
    auto range = _role_data->_candidate_time;
    uint32_t time = absl::uniform_int_distribution<uint32_t>(range.first, range.second)(_role_data->_gen);
    _role_data->_timer->StartVoteTimer(time);

    // already vote to other node 
    if (_role_data->_voted_for_id != 0 && _role_data->_voted_for_id != _role_data->_cur_node_id) {
        _role_data->_voted_for_id = 0;
        return;
    }

    // vote to myself
    _role_data->_voted_for_id = _role_data->_cur_node_id;

    // send all node to get vote
    VoteRequest request;
    request.set_term(_role_data->_current_term + 1);
    request.set_candidate_id(_role_data->_cur_node_id);
    request.set_last_term(_role_data->_current_term);
    request.set_last_index(_role_data->_newest_index);
    _role_data->_node_manager->SendVoteToAll(request);
    base::LOG_DEBUG("candidate time out, send context : %s", request.DebugString().c_str());
}

void CCandidateRole::HeartBeatTimerOut() {
    // stop heart timer
    _role_data->_timer->StopHeartTimer();
}
