#include "Log.h"
#include "INode.h"
#include "ITimer.h"
#include "IClient.h"
#include "RoleData.h"
#include "FollowerRole.h"
#include "absl/time/time.h"
#include "absl/time/clock.h"

using namespace raft;

CFollowerRole::CFollowerRole(std::shared_ptr<CRoleData>& role_data) :
    CRole(role_data),
    _turn_time(0),
    _first_heart(true) {

}

CFollowerRole::~CFollowerRole() {

}

ROLE_TYPE CFollowerRole::GetRole() {
    return follower_role;
}

void CFollowerRole::ItsMyTurn() {
    _role_data->_voted_for_id = 0;
    // stop heart timer
    _role_data->_timer->ResetTimer();
    // reset candidate timer 
    auto range = _role_data->_candidate_time;
    uint32_t time = absl::uniform_int_distribution<uint32_t>(range.first, range.second)(_role_data->_gen);
    _role_data->_timer->StartVoteTimer(time);
}

void CFollowerRole::RecvVoteRequest(std::shared_ptr<CNode>& node, VoteRequest& vote_request) {
    base::LOG_DEBUG("follower recv vote request from node, %s, context : %s",
        node->GetNetHandle().c_str(), vote_request.DebugString().c_str());

    VoteResponse response;
    response.set_vote_granted(true);
    // compare term
    if (vote_request.term() < _role_data->_current_term) {
        response.set_vote_granted(false);
    }

    // compare prev log index and term
    if (_role_data->_entries_map.size() > 0 && response.vote_granted()) {
        auto last_entries = _role_data->_entries_map.rbegin();
        Entries& prev_entries = last_entries->second;
        if (prev_entries._index > vote_request.last_index() ||
            prev_entries._term > vote_request.last_term()) {
            response.set_vote_granted(false);
        }
    }

    if (response.vote_granted()) {
        auto range = _role_data->_candidate_time;
        uint64_t now = absl::ToUnixMillis(absl::Now());
        if (_role_data->_voted_for_id != 0 && now - _turn_time <= range.first) {
            response.set_vote_granted(false);

        }
        else {
            _role_data->_voted_for_id = vote_request.candidate_id();
            // reset candidate timer 
            uint32_t time = absl::uniform_int_distribution<uint32_t>(range.first, range.second)(_role_data->_gen);
            _role_data->_timer->StartVoteTimer(time);
        }
        _turn_time = now;
    }

    response.set_term(_role_data->_current_term);
    node->SendVoteResponse(response);
}

void CFollowerRole::RecvHeartBeatRequest(std::shared_ptr<CNode>& node, HeartBeatResquest& heart_request) {
    base::LOG_DEBUG("follower recv heart request from node, %s, context : %s",
        node->GetNetHandle().c_str(), heart_request.DebugString().c_str());

    // reset candidate timer
    _role_data->_timer->ResetTimer();
    // reset candidate timer 
    auto range = _role_data->_candidate_time;
    uint32_t time = absl::uniform_int_distribution<uint32_t>(range.first, range.second)(_role_data->_gen);
    _role_data->_timer->StartVoteTimer(time);

    // set leader net handle
    if (_role_data->_leader_net_handle != node->GetNetHandle()) {
        _role_data->_leader_net_handle = node->GetNetHandle();
    }
    _role_data->_current_term = heart_request.term();

    HeartBeatResponse response;
    response.set_success(true);
    response.set_term(_role_data->_current_term);
    // compare term
    if (heart_request.term() < _role_data->_current_term) {
        response.set_success(false);
    }
    // compare prev log term
    if (_role_data->_entries_map.size() > 0 && response.success()) {
        auto last_entries = _role_data->_entries_map.rbegin();
        Entries& prev_entries = last_entries->second;

        // if index is the same but term is different
        if (prev_entries._term != heart_request.prev_log_term()
            || prev_entries._index != heart_request.prev_log_index()) {
            response.set_success(false);
            response.set_next_index(prev_entries._index);
            // delete after entries
            auto iter = _role_data->_entries_map.find(prev_entries._index);
            while (iter != _role_data->_entries_map.end()) {
                iter = _role_data->_entries_map.erase(iter);
            }
        }
    }

    response.set_next_index(_role_data->_newest_index + 1);
    // can't recv that entries
    if (!response.success()) {
        node->SendHeartResponse(response);
        return;
    }

    if (_first_heart) {
        _first_heart = false;
        _role_data->_role_change_call_back(follower_role, node->GetNetHandle());
    }

    // apply entries
    auto leader_commit = heart_request.leader_commit();
    if (_role_data->_last_applied < leader_commit) {
        auto iter = _role_data->_entries_map.find(leader_commit);
        if (iter != _role_data->_entries_map.end()) {
            auto start = _role_data->_last_applied ? _role_data->_entries_map.find(_role_data->_last_applied + 1) : _role_data->_entries_map.begin();
            while (start != _role_data->_entries_map.end() && start->first <= leader_commit) {
                _role_data->_commit_entries_call_back(start->second);
                start++;
            }
        }
        _role_data->_last_applied = leader_commit;
    }

    // copy entries
    for (size_t i = 0; i < heart_request.entries_size(); i++) {
        auto entries_data = heart_request.entries(i);
        EntriesRef ref((char*)entries_data.c_str(), entries_data.length());
        _role_data->_entries_map[ref.GetIndex()] = ref.GetEntries();
        _role_data->_newest_index = ref.GetIndex();
    }

    response.set_next_index(_role_data->_newest_index + 1);
    node->SendHeartResponse(response);
}

void CFollowerRole::RecvVoteResponse(std::shared_ptr<CNode>& node, VoteResponse& vote_response) {
    // do nothing
    base::LOG_DEBUG("follower recv vote response from node, %s, context : %s",
        node->GetNetHandle().c_str(), vote_response.DebugString().c_str());
}

void CFollowerRole::RecvHeartBeatResponse(std::shared_ptr<CNode>& node, HeartBeatResponse& heart_response) {
    // do nothing
    base::LOG_DEBUG("follower recv heart response from node, %s, context : %s",
        node->GetNetHandle().c_str(), heart_response.DebugString().c_str());
}

void CFollowerRole::RecvClientRequest(std::shared_ptr<CClient>& client, ClientRequest& request) {
    base::LOG_DEBUG("follower recv client request from client, %s, context : %s",
        client->GetNetHandle().c_str(), request.DebugString().c_str());

    // tell client send to leader.
    ClientResponse response;
    response.set_ret_code(not_leader);
    response.set_leader_net_handle(_role_data->_leader_net_handle);
    client->SendToClient(response);
}

void CFollowerRole::CandidateTimeOut() {
    base::LOG_DEBUG("follower time out to be a candidate");

    _first_heart = true;
    // to be a candidate
    _role_data->_role_change_call_back(candidate_role, "");
}

void CFollowerRole::HeartBeatTimerOut() {
    // stop heart timer
    _role_data->_timer->StopHeartTimer();
}
