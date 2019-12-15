#include "CandidateRole.h"
#include "INode.h"
#include "ITimer.h"
#include "RaftMediator.h"

using namespace raft;

CCandidateRole::CCandidateRole(std::shared_ptr<CRoleData>& role_data, std::shared_ptr<CTimer>& timer, CRaftMediator* mediator) : CRole(role_data, timer, mediator) {

}

CCandidateRole::~CCandidateRole() {

}

ROLE_TYPE CCandidateRole::GetRole() {
    return candidate_role;
}

void CCandidateRole::RecvVote(std::shared_ptr<CNode> node, VoteRequest& vote_request) {
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

    node->SendVoteResponse(response);
}

void CCandidateRole::RecvHeartBeat(std::shared_ptr<CNode> node, HeartBeatResquest& heart_request) {
    // change role to follower
    _role_data->_role_change_call_back(follower_role);
    // recv that request again
    _role_data->_raft_mediator->RecvHeartBeat(node, heart_request);
}

void CCandidateRole::RecvVoteResponse(std::shared_ptr<CNode> node, VoteResponse& vote_response) {
    // do nothing
}

void CCandidateRole::RecvHeartBeatResponse(std::shared_ptr<CNode> node, HeartBeatResponse& heart_response) {
    // do nothing
}

void CCandidateRole::CandidateTimeOut() {
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

void CCandidateRole::HeartBeatTimerOut() {
    // stop heart timer
    _role_data->_timer->StopHeartTimer();
}
