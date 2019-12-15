#include "FollowerRole.h"
#include "INode.h"
#include "ITimer.h"
#include "RaftMediator.h"

using namespace raft;

CFollowerRole::CFollowerRole(std::shared_ptr<CRoleData>& role_data, std::shared_ptr<CTimer>& timer, CRaftMediator* mediator) : CRole(role_data, timer, mediator) {

}

CFollowerRole::~CFollowerRole() {

}

ROLE_TYPE CFollowerRole::GetRole() {
    return follower_role;
}

void CFollowerRole::RecvVote(std::shared_ptr<CNode> node, VoteRequest& vote_request) {
    VoteResponse response;
    if (vote_request.last_term() < _role_data->_current_term) {
        response.set_vote_granted(false);
        
    }
    response.set_vote_granted(true);
    response.set_term(_role_data->_current_term);
    node->SendVoteResponse(response);
}

void CFollowerRole::RecvHeartBeat(std::shared_ptr<CNode> node, HeartBeatResquest& heart_request) {
    HeartBeatResponse response;
    response.set_success(true);
    response.set_term(_role_data->_current_term);
    // compare term
    if (heart_request.term() < _role_data->_current_term) {
        response.set_success(false);
    }
    // compare prev log index and term
    auto last_entries = _role_data->_entries_map.end()--;
    Entries& prev_entries = last_entries->second;
    if (prev_entries._index != heart_request.prev_log_term() || 
        prev_entries._term != heart_request.prev_log_term()) {
        response.set_success(false);
    }
    // can't recv that entries
    if (!response.success()) {
        node->SendHeartResponse(response);
        return;
    }

    // apply entries
    uint64_t leader_commit = heart_request.leader_commit();
    if (leader_commit != 0) {
        for (auto iter = _role_data->_entries_map.begin(); iter != _role_data->_entries_map.end();) {
            if (iter->first > leader_commit) {
                break;
            }
            _role_data->_commit_entries_call_back(iter->second);
            iter = _role_data->_entries_map.erase(iter);
        }
    }
    
    // copy entries
    for (size_t i = 0; i < heart_request.entries_size(); i++) {
        auto entries_data = heart_request.entries(i);
        EntriesRef ref((char*)entries_data.c_str(), entries_data.length());
        _role_data->_entries_map[ref.GetIndex()] = ref.GetEntries();
    }
    _role_data->_current_term = heart_request.term();
    _role_data->_newest_index = _role_data->_entries_map.begin()->first;
}

void CFollowerRole::RecvVoteResponse(std::shared_ptr<CNode> node, VoteResponse& vote_response) {
    // do nothing
}

void CFollowerRole::RecvHeartBeatResponse(std::shared_ptr<CNode> node, HeartBeatResponse& heart_response) {
    // do nothing
}

void CFollowerRole::CandidateTimeOut() {
    // to be a candidate
    _role_data->_role_change_call_back(candidate_role);

    // send all node to get vote
    VoteRequest request;
    request.set_term(_role_data->_current_term + 1);
    request.set_candidate_id(_role_data->_raft_mediator->GetId());
    request.set_last_term(_role_data->_current_term);
    request.set_last_index(_role_data->_newest_index);
    _role_data->_raft_mediator->SendVoteToAll(request);
    // reset candidate timer 
    uint32_t time = absl::uniform_int_distribution<uint32_t>(150, 300)(_role_data->_gen);
    _role_data->_timer->StartVoteTimer(time);
}

void CFollowerRole::HeartBeatTimerOut() {
    // stop heart timer
    _role_data->_timer->StopHeartTimer();
}
