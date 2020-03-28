#include "Log.h"
#include "INode.h"
#include "ITimer.h"
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
    CandidateTimeOut();
}

void CCandidateRole::RecvVoteRequest(std::shared_ptr<CNode>& node, VoteRequest& vote_request) {
    base::LOG_DEBUG("candidate recv vote request from node, %s, context : %s", 
                node->GetNetHandle().c_str(), vote_request.DebugString().c_str());

    VoteResponse response;
    response.set_term(_role_data->_current_term);
    response.set_vote_granted(false);
    // refuse vote
    node->SendVoteResponse(response);
}

void CCandidateRole::RecvHeartBeatRequest(std::shared_ptr<CNode>& node, HeartBeatResquest& heart_request) {
    base::LOG_DEBUG("candidate recv heart request from node, %s, context : %s", 
                node->GetNetHandle().c_str(), heart_request.DebugString().c_str());

    HeartBeatResponse response;
    response.set_success(true);
    if (heart_request.term() < _role_data->_current_term) {
        response.set_success(false);
    }

    // compare prev log index and term
    if (_role_data->_entries_map.size() > 0 && response.success()) {
        auto last_entries = _role_data->_entries_map.end()--;
        Entries& prev_entries = last_entries->second;
        if (prev_entries._term > heart_request.prev_log_term() ||
            prev_entries._index > heart_request.prev_log_index()) {
            response.set_success(false);
        }
    }

    if (response.success()) {
        // set leader node
        _role_data->_leader_node = node;
        // change role to follower
        _role_data->_role_change_call_back(follower_role, node->GetNetHandle());
        // change to follower recv this request again
        _role_data->_recv_heart_again(node, heart_request);
    }
}

void CCandidateRole::RecvVoteResponse(std::shared_ptr<CNode>& node, VoteResponse& vote_response) {
    base::LOG_DEBUG("candidate recv vote response from node, %s, context : %s", 
                node->GetNetHandle().c_str(), vote_response.DebugString().c_str());

    if (vote_response.vote_granted()) {
        _role_data->_vote_num++;
    }
    _role_data->_current_term += 1;
    if (_role_data->_vote_num >= _role_data->_node_manager->GetNodeCount() / 2) {
        _role_data->_vote_num = 0;
        // to be a leader
        _role_data->_role_change_call_back(leader_role, _role_data->_cur_net_handle);
    }
}

void CCandidateRole::RecvHeartBeatResponse(std::shared_ptr<CNode>& node, HeartBeatResponse& heart_response) {
    // do nothing
    base::LOG_DEBUG("candidate recv heart response from node, %s, context : %s", 
                node->GetNetHandle().c_str(), heart_response.DebugString().c_str());
}

void CCandidateRole::RecvEntriesRequest(std::shared_ptr<CNode>& node, EntriesRequest& request) {
    base::LOG_DEBUG("candidate recv client request from node, %s, context : %s", 
                node->GetNetHandle().c_str(), request.DebugString().c_str());

    // cache request, send to leader when see a leader.
    _role_data->_entries_request_cache.push_back(request);
}

void CCandidateRole::CandidateTimeOut() {
    // reset vote info
    _role_data->_vote_num = 0;
    _role_data->_voted_for_id = 0;
    // reset candidate timer 
    auto range = _role_data->_candidate_time;
    uint32_t time = absl::uniform_int_distribution<uint32_t>(range.first, range.second)(_role_data->_gen);
    _role_data->_timer->StartVoteTimer(time);

    // vote to myself
    _role_data->_voted_for_id = _role_data->_cur_node_id;
    _role_data->_vote_num++;

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
