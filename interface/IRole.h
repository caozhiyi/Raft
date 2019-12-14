#ifndef RAFT_INTERFACE_ROLE
#define RAFT_INTERFACE_ROLE

#include "message.pb.h"
#include "Entries.h"

namespace raft {
    enum ROLE_TYPE {
        leader_role    = 1,
        candidate_role = 2,
        follower_role  = 3
    };

    class CNode;
    class CRole {
    public:
        CRole() {}
        virtual ~CRole() {}
        // get role type
        virtual ROLE_TYPE GetRole() = 0;
        // need to vote?
        virtual bool RecvVoteRequest(VoteRequest& vote_request) = 0;
        // get a heart message
        virtual void RecvHeartBeatRequest(HeartBeatResquest& heart_request) = 0;
        // need to vote?
        virtual bool RecvVoteresponse(VoteResponse& vote_response) = 0;
        // get a heart message
        virtual void RecvHeartBeatresponse(HeartBeatResponse& heart_response) = 0;
        // when candidate timer out. follower and candidate
        virtual void CandidateTimeOut() {}
        // when heart beat timer out
        virtual void HeartBeatTimerOut() {}
        // virtual 
    private:
        uint32_t                _current_term;
        uint32_t                _voted_for_id;
        std::vector<Entries>    _entries_vec;

        // current newest enteies index
        uint64_t                _newest_index;
        // last to commit to status machine entries index
        uint64_t                _last_applied;

        // all node vector
        std::vector<std::share_ptr<CNode>> _node_vec;
    };
}

#endif