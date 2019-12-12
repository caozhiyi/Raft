#ifndef RAFT_INTERFACE_TIMER
#define RAFT_INTERFACE_TIMER

#include "LogRef.h"

namespace raft {
    enum ROLE_TYPE {
        leader_role    = 1,
        candidate_role = 2,
        follower_role  = 3
    };

    class CRole {
    public:
        CRole() {}
        virtual ~CRole() {}
        // get role type
        virtual ROLE_TYPE GetRole() = 0;
        // need to vote?
        virtual bool ToVote(uint32_t term, uint64_t index) = 0;
        // get a heart message
        virtual void RecvHeartBeat() = 0;
        // when candidate timer out. follower and candidate
        virtual void CandidateTimeOut() {}
        // when heart beat timer out
        virtual void HeartBeatTimerOut() {}
    };
}

#endif