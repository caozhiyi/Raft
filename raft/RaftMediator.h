#ifndef RAFT_RAFT_RAFTMEDIATOR
#define RAFT_RAFT_RAFTMEDIATOR

#include <memory>
#include <functional>
#include <map>

#include "IRole.h"
#include "Single.h"
#include "RoleData.h"
#include "message.pb.h"
#include "absl/strings/string_view.h"
#include "absl/functional/function_ref.h"

namespace raft {
    
    class CNet;
    class CNode;
    class CTimer;
    class CConfig;
    class Entries;
    class CMountClient;
    class CNodeManager;
    class CClientManager;
    class CCommitEntries;
    class CRaftMediator : public base::CSingle<CRaftMediator> {
    public:
        CRaftMediator();
        ~CRaftMediator();

        // start work
        void Start(const std::string& config_file);
        void Join();
        void Dealloc();
  
        // get currnet node id
        uint32_t GetId();

        // commit entries
        void CommitEntries(Entries& entries);
        // role changed
        void ChangeRole(ROLE_TYPE type, const std::string& net_handle);

        // send vote to all node
        void SendVoteToAll(VoteRequest& request);
        // send heart beat to all node
        void SendHeartBeatToAll(HeartBeatResquest& request);

        // recv heart request again
        void RecvHeartBeat(std::shared_ptr<CNode>& node, HeartBeatResquest& request);

        // set commit call back
        void SetCommitCallBack(absl::FunctionRef<void(std::string)> func);

        // get all node
        const std::map<std::string, std::shared_ptr<CNode>>& GetAllNode();

        // get node number
        uint32_t GetNodeCount();

        // get heart time
        uint32_t GetHeartTime();

        // get current node net handle
        std::string GetCurNodeHandle();

    private:
        uint32_t                        _id;
        std::shared_ptr<CNet>           _net;
        std::shared_ptr<CTimer>         _timer;
        std::shared_ptr<CCommitEntries> _commit_entries;
        std::shared_ptr<CConfig>        _config;
        std::shared_ptr<CMountClient>   _mount_client;

        std::string                     _cur_node_net;
        // raft node manager
        std::shared_ptr<CNodeManager>   _node_manager;
        // client manager
        std::shared_ptr<CClientManager> _client_manager;

        // role about
        std::shared_ptr<CRole>          _current_role;
        std::shared_ptr<CRole>          _leader_role;
        std::shared_ptr<CRole>          _candidate_role;
        std::shared_ptr<CRole>          _follower_role;
    
        // commit entries call back
        std::function<void(std::string)>              _commit_call_back;
    };
}

#endif