#include "INode.h"
#include "IClient.h"
#include "LeaderRole.h"
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
    // set leader net handle
    if (_role_data->_net_handle != node->GetNetHandle()) {
        _role_data->_net_handle = node->GetNetHandle();
    }
    // change to follower recv this request again
    _role_data->_raft_mediator->RecvHeartBeat(node, heart_request);
}

void CLeaderRole::RecvVoteResponse(std::shared_ptr<CNode>& node, VoteResponse& vote_response) {
    // do nothing
}

void CLeaderRole::RecvHeartBeatResponse(std::shared_ptr<CNode>& node, HeartBeatResponse& heart_response) {
    // send response to client
    node->SetNextIndex(heart_response.next_index());
    if (heart_response.success()) {
        _role_data->_heart_success_num++;
        _role_data->_max_match_index = std::min(_role_data->_max_match_index, heart_response.next_index() - 1);
    }
    
    // commit entries success
    if (_role_data->_heart_success_num > _role_data->_raft_mediator->GetNodeCount() / 2) {
        auto iter = _role_data->_entries_map.find(_role_data->_prev_match_index);
        ClientResponse response;
        response.set_ret_code(success);
        while (iter != _role_data->_entries_map.end() && iter->first <= _role_data->_max_match_index) {
            // commit entries
            _role_data->_commit_entries_call_back(iter->second);

            // send response to client
            auto client_iter = _client_net_handle_map.find(iter->first);
            if (client_iter != _client_net_handle_map.end()) {
                client_iter->second->SendToClient(response);
            }
        }
    }
}

void CLeaderRole::RecvClientRequest(std::shared_ptr<CClient>& client, ClientRequest& request) {
    // get entries from client
    // create entries
    _role_data->_newest_index++;
    Entries entries;
    entries._entries = request.entries();
    entries._index = _role_data->_newest_index;
    entries._term = _role_data->_current_term;

    // insert entries
    _role_data->_entries_map[entries._index] = entries;
    _client_net_handle_map[entries._index] = client;
}

void CLeaderRole::CandidateTimeOut() {
    // do nothing
    _role_data->_voted_for_id = 0;
}

void CLeaderRole::HeartBeatTimerOut() {
    _role_data->_heart_success_num = 0;
    // send heart request to all node again
    HeartBeatResquest request;
    request.set_term(_role_data->_current_term);
    request.set_leader_id(_role_data->_raft_mediator->GetId());

    // add last entries info
    if (_role_data->_entries_map.size() > 0) {
        auto iter = _role_data->_entries_map.end()--;
        request.set_prev_log_index(iter->first);
        request.set_prev_log_term(iter->second._term);
    }

    request.set_leader_commit(_role_data->_last_applied);
    
    // send request to all node
    auto node_map = _role_data->_raft_mediator->GetAllNode();
    for (auto node_iter = node_map.begin(); node_iter != node_map.end(); ++node_iter) {
        uint64_t next_index = node_iter->second->GetNextIndex();
        if (next_index > 0) {
            // add new entries
            auto iter = _role_data->_entries_map.find(_role_data->_last_applied);
            if (iter != _role_data->_entries_map.end()) {
                iter++;
                while (iter != _role_data->_entries_map.end()) {
                    EntriesRef ref(iter->second);
                    request.add_entries(ref.GetData());
                }
            }
        }
        node_iter->second->SendHeartRequest(request);
        request.clear_entries();
    }
}