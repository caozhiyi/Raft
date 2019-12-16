#ifndef RAFT_RAFT_LEADERROLE
#define RAFT_RAFT_LEADERROLE

#include "IRole.h"

namespace raft {

    class CLeaderRole : public CRole {
    public:
        CLeaderRole(std::shared_ptr<CRoleData>& role_data, std::shared_ptr<CTimer>& timer, CRaftMediator* mediator);
        virtual ~CLeaderRole();
        // get role type
        ROLE_TYPE GetRole();
        // need to vote?
        void RecvVoteRequest(std::shared_ptr<CNode> node, VoteRequest& vote_request);
        // get a heart message
        void RecvHeartBeatRequest(std::shared_ptr<CNode> node, HeartBeatResquest& heart_request);
        // get a vote response?
        void RecvVoteResponse(std::shared_ptr<CNode> node, VoteResponse& vote_response);
        // get a heart message
        void RecvHeartBeatResponse(std::shared_ptr<CNode> node, HeartBeatResponse& heart_response);
        // when candidate timer out. follower and candidate
        void CandidateTimeOut();
        // when heart beat timer out
        void HeartBeatTimerOut();
    };
}

#endif