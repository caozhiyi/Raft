#ifndef RAFT_RAFT_ROLEDATA
#define RAFT_RAFT_ROLEDATA

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

#include "IRole.h"
#include "Entries.h"
#include "message.pb.h"
#include "absl/random/random.h"
#include "absl/functional/function_ref.h"

namespace raft {

    class CNode;
    class CTimer;
    class CNodeManager;
    class CRoleData {
    public:
        std::shared_ptr<CTimer>       _timer;
        std::shared_ptr<CNodeManager> _node_manager;
        uint32_t                      _current_term;
        uint32_t                      _voted_for_id;
        std::map<uint64_t, Entries>   _entries_map;
        // random about
        absl::BitGen                  _gen;
        // current newest entries index
        uint64_t                      _newest_index;
        // last to commit to status machine entries index
        uint64_t                      _last_applied;

        uint32_t                      _vote_num;
        uint32_t                      _heart_success_num;

        uint64_t                      _max_match_index;
        uint64_t                      _prev_match_index;

        // leader net handle
        std::string                   _leader_net_handle;
        // current node net handle
        std::string                   _cur_net_handle;
        // current node id          
        uint32_t                      _cur_node_id;
        // heart time
        uint32_t                      _heart_time;
        // candidate time
        std::pair<uint32_t, uint32_t> _candidate_time;

        std::function<void(ROLE_TYPE, const std::string&)>               _role_change_call_back;
        std::function<void(Entries&)>                                    _commit_entries_call_back;
        std::function<void(std::shared_ptr<CNode>&, HeartBeatResquest&)> _recv_heart_again;

    public:
        CRoleData() : 
            _timer(nullptr),
            _node_manager(nullptr), 
            _current_term(0),
            _voted_for_id(0),
            _newest_index(0),
            _last_applied(0),
            _vote_num(0), 
            _heart_success_num(0),
            _max_match_index(0),
            _prev_match_index(0),
            _cur_node_id(0),
            _heart_time(0) {}
    };
}

#endif