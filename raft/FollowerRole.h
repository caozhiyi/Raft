#ifndef RAFT_RAFT_CFOLLOWERROLE
#define RAFT_RAFT_CFOLLOWERROLE

#include "IRole.h"

namespace raft {

    class CRoleData;
    class CFollowerRole : public CRole {
    public:
        CFollowerRole(std::shared_ptr<CRoleData>& role_data, std::shared_ptr<CTimer>& timer, CRaftMediator* mediator);
        virtual ~CFollowerRole();
        // get role type
        ROLE_TYPE GetRole();
        // need to vote?
        void RecvVote(std::shared_ptr<CNode> node, VoteRequest& vote_request);
        // get a heart message
        void RecvHeartBeat(std::shared_ptr<CNode> node, HeartBeatResquest& heart_request);
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