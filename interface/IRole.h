#ifndef RAFT_INTERFACE_ROLE
#define RAFT_INTERFACE_ROLE

#include <memory>
#include <vector>
#include <functional>
#include "Entries.h"
#include "message.pb.h"
#include "RoleData.h"
#include "absl/functional/function_ref.h"

namespace raft {

    class CTimer;
    class CNode;
    class CRole {
    public:
        CRole(std::shared_ptr<CRoleData>& role_data, std::shared_ptr<CTimer>& timer, CRaftMediator* mediator) : _role_data(role_data) {
            _role_data->_raft_mediator = mediator;
            _role_data->_timer = timer;
        }
        virtual ~CRole() {}
        // get role type
        virtual ROLE_TYPE GetRole() = 0;
        // need to vote?
        virtual void RecvVoteRequest(std::shared_ptr<CNode> node, VoteRequest& vote_request) = 0;
        // get a heart message
        virtual void RecvHeartBeatRequest(std::shared_ptr<CNode> node, HeartBeatResquest& heart_request) = 0;
        // get a vote response?
        virtual void RecvVoteResponse(std::shared_ptr<CNode> node, VoteResponse& vote_response) = 0;
        // get a heart message
        virtual void RecvHeartBeatResponse(std::shared_ptr<CNode> node, HeartBeatResponse& heart_response) = 0;
        // when candidate timer out. follower and candidate
        virtual void CandidateTimeOut() {}
        // when heart beat timer out
        virtual void HeartBeatTimerOut() {}
        // set role changed call back
        virtual void SetRoleChangeCalBack(absl::FunctionRef<void(ROLE_TYPE)> func) {
            _role_data->_role_change_call_back = func;
        }
        // set commitentries call back
        virtual void SetCommitEntriesCalBack(absl::FunctionRef<void(Entries&)> func) {
            _role_data->_commit_entries_call_back = func;
        }

    protected:
        std::shared_ptr<CRoleData>      _role_data;
    };
}

#endif