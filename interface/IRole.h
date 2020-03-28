#ifndef RAFT_INTERFACE_ROLE
#define RAFT_INTERFACE_ROLE

#include <memory>
#include <vector>
#include <functional>
#include "message.pb.h"

namespace raft {

    enum ROLE_TYPE {
        leader_role = 1,
        candidate_role = 2,
        follower_role = 3
    };
    
    class CNode;
    class CTimer;
    class CClient;
    class CRoleData;
    class CRole {
    public:
        CRole(std::shared_ptr<CRoleData>& role_data) : _role_data(role_data) {}
        virtual ~CRole() {}
        // get role type
        virtual ROLE_TYPE GetRole() = 0;
        // role init when role change
        virtual void ItsMyTurn() = 0;
        // need to vote?
        virtual void RecvVoteRequest(std::shared_ptr<CNode>& node, VoteRequest& vote_request) = 0;
        // get a heart message
        virtual void RecvHeartBeatRequest(std::shared_ptr<CNode>& node, HeartBeatResquest& heart_request) = 0;
        // get a vote response?
        virtual void RecvVoteResponse(std::shared_ptr<CNode>& node, VoteResponse& vote_response) = 0;
        // get a heart message
        virtual void RecvHeartBeatResponse(std::shared_ptr<CNode>& node, HeartBeatResponse& heart_response) = 0;
        // get a entries request
        virtual void RecvEntriesRequest(std::shared_ptr<CNode>& node, EntriesRequest& request) = 0;
        // when candidate timer out. follower and candidate
        virtual void CandidateTimeOut() {}
        // when heart beat timer out
        virtual void HeartBeatTimerOut() {}

    protected:
        std::shared_ptr<CRoleData>      _role_data;
    };
}

#endif