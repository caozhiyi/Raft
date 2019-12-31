#include "INode.h"
#include "ITimer.h"
#include "IClient.h"
#include "RoleData.h"
#include "FollowerRole.h"

using namespace raft;

CFollowerRole::CFollowerRole(std::shared_ptr<CRoleData>& role_data) : CRole(role_data) {

}

CFollowerRole::~CFollowerRole() {

}

ROLE_TYPE CFollowerRole::GetRole() {
    return follower_role;
}

void CFollowerRole::ItsMyTurn() {
    // stop heart timer
    _role_data->_timer->StopHeartTimer();
}

void CFollowerRole::RecvVoteRequest(std::shared_ptr<CNode>& node, VoteRequest& vote_request) {
    VoteResponse response;
    if (vote_request.last_term() < _role_data->_current_term) {
        response.set_vote_granted(false);
        
    }
    response.set_vote_granted(true);
    response.set_term(_role_data->_current_term);
    node->SendVoteResponse(response);
}

void CFollowerRole::RecvHeartBeatRequest(std::shared_ptr<CNode>& node, HeartBeatResquest& heart_request) {
    HeartBeatResponse response;
    response.set_success(true);
    response.set_term(_role_data->_current_term);
    // compare term
    if (heart_request.term() < _role_data->_current_term) {
        response.set_success(false);
    }
    // compare prev log term
    auto last_entries = _role_data->_entries_map.end()--;
    Entries& prev_entries = last_entries->second;
    if (prev_entries._term != heart_request.prev_log_term()) {
        response.set_next_index(prev_entries._index);
        response.set_success(false);
    }

    // if index is the same but term is different
    if (prev_entries._term != heart_request.prev_log_term() 
        && prev_entries._index == heart_request.prev_log_index()) {
        response.set_success(false);
        response.set_next_index(prev_entries._index);
        // delete after entries
        auto iter = _role_data->_entries_map.find(prev_entries._index);
        while (iter != _role_data->_entries_map.end()) {
            iter = _role_data->_entries_map.erase(iter);
        }
    }

    // can't recv that entries
    if (!response.success()) {
        node->SendHeartResponse(response);
        return;
    }

    // set leader net handle
    if (_role_data->_leader_net_handle != node->GetNetHandle()) {
        _role_data->_leader_net_handle = node->GetNetHandle();
    }

    // apply entries
    auto leader_commit = heart_request.leader_commit();
    if (leader_commit > 0) {
        auto iter = _role_data->_entries_map.find(leader_commit);
        if (iter != _role_data->_entries_map.end()) {
            auto start = _role_data->_entries_map.begin();
            while (start != _role_data->_entries_map.end() && start->first != leader_commit) {
                _role_data->_commit_entries_call_back(start->second);
                start = _role_data->_entries_map.erase(start);
            }
        }
    }

    // copy entries
    for (size_t i = 0; i < heart_request.entries_size(); i++) {
        auto entries_data = heart_request.entries(i);
        EntriesRef ref((char*)entries_data.c_str(), entries_data.length());
        _role_data->_entries_map[ref.GetIndex()] = ref.GetEntries();
    }
    _role_data->_current_term = heart_request.term();
    _role_data->_newest_index = _role_data->_entries_map.end()->first;
    response.set_next_index(_role_data->_newest_index + 1);
}

void CFollowerRole::RecvVoteResponse(std::shared_ptr<CNode>& node, VoteResponse& vote_response) {
    // do nothing
}

void CFollowerRole::RecvHeartBeatResponse(std::shared_ptr<CNode>& node, HeartBeatResponse& heart_response) {
    // do nothing
}

void CFollowerRole::RecvClientRequest(std::shared_ptr<CClient>& client, ClientRequest& request) {
    // tell client send to leader.
    ClientResponse response;
    response.set_ret_code(not_leader);
    response.set_leader_net_handle(_role_data->_leader_net_handle);
    client->SendToClient(response);
}

void CFollowerRole::CandidateTimeOut() {
    // already vote to other node 
    if (_role_data->_voted_for_id != 0) {
        _role_data->_voted_for_id = 0;
        return;
    }

    // vote to myself
    _role_data->_voted_for_id = _role_data->_cur_node_id;

    // to be a candidate
    _role_data->_role_change_call_back(candidate_role, "");

    // reset candidate timer 
    uint32_t time = absl::uniform_int_distribution<uint32_t>(150, 300)(_role_data->_gen);
    _role_data->_timer->StartVoteTimer(time);
}

void CFollowerRole::HeartBeatTimerOut() {
    // stop heart timer
    _role_data->_timer->StopHeartTimer();
}
