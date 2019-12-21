#ifndef RAFT_RAFT_ROLEDATA
#define RAFT_RAFT_ROLEDATA

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

#include "Entries.h"
#include "message.pb.h"
#include "absl/random/random.h"
#include "absl/functional/function_ref.h"

namespace raft {

    enum ROLE_TYPE {
        leader_role = 1,
        candidate_role = 2,
        follower_role = 3
    };

    class CTimer;
    class CRaftMediator;
    class CRoleData {
    public:
        std::shared_ptr<CTimer>      _timer;
        CRaftMediator*               _raft_mediator;
        uint32_t                     _current_term;
        uint32_t                     _voted_for_id;
        std::map<uint64_t, Entries>  _entries_map;
        // random about
        absl::BitGen                 _gen;
        // current newest entries index
        uint64_t                     _newest_index;
        // last to commit to status machine entries index
        uint64_t                     _last_applied;

        uint32_t                     _vote_num;
        uint32_t                     _heart_success_num;

        uint64_t                     _max_match_index;
        uint64_t                     _prev_match_index;

        // leader net handle
        std::string                  _net_handle;

        std::function<void(ROLE_TYPE)> _role_change_call_back;
        std::function<void(Entries&)>  _commit_entries_call_back;
    public:
        CRoleData() : _timer(nullptr),
            _raft_mediator(nullptr), 
            _current_term(0),
            _voted_for_id(0),
            _newest_index(0),
            _last_applied(0),
            _vote_num(0), 
            _heart_success_num(0),
            _max_match_index(0),
            _prev_match_index(0) {

        }
    };
}

#endif