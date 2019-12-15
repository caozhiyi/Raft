#ifndef RAFT_RAFT_RAFTMEDIATOR
#define RAFT_RAFT_RAFTMEDIATOR

#include <memory>
#include <functional>
#include <map>
#include "message.pb.h"
#include "IRole.h"
#include "RoleData.h"
#include "absl/strings/string_view.h"
#include "absl/functional/function_ref.h"

namespace raft {
    
    class CNet;
    class CNode;
    class CTimer;
    class Entries;
    class CCommitEntries;
    class CRaftMediator {
    public:
        CRaftMediator();
        ~CRaftMediator();
        // get currnet node id
        uint32_t GetId();

        // connect about
        void ConnectedNode(absl::string_view ip, uint16_t port);
        void DisConnectedNode(absl::string_view ip, uint16_t port);
        // commit entries
        void CommitEntries(Entries& entries);
        // role changed
        void ChangeRole(ROLE_TYPE type);

        // send vote to all node
        void SendVoteToAll(VoteRequest& request);
        // send heart beat to all node
        void SendHeartBeatToAll(HeartBeatResquest& request);

        // recv heart request again
        void RecvHeartBeat(std::shared_ptr<CNode>& node, HeartBeatResquest& request);

        // set commit call back
        void SetCommitCallBack(absl::FunctionRef<void(std::string)> func);

    private:
        uint32_t                        _id;
        std::shared_ptr<CNet>           _net;
        std::shared_ptr<CTimer>         _timer;
        std::shared_ptr<CCommitEntries> _commit_entries;

        // role about
        std::shared_ptr<CRole>          _current_role;
        std::shared_ptr<CRole>          _leader_role;
        std::shared_ptr<CRole>          _candidate_role;
        std::shared_ptr<CRole>          _follower_role;
        // all node
        std::map<std::string, std::shared_ptr<CNode>> _node_map;
    
        // commit entries call back
        std::function<void(std::string)>              _commit_call_back;
    };
}

#endif