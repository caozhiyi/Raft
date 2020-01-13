#ifndef RAFT_RAFT_NODEMANAGERIMPL
#define RAFT_RAFT_NODEMANAGERIMPL

#include <unordered_map>
#include "INodeManager.h"

namespace raft {

    class CNet;
    class CNode;
    class CRole;
    class CNodeManagerImpl : public CNodeManager {
    public:
        CNodeManagerImpl(std::shared_ptr<CNet>& net, const std::string& net_handle);
        ~CNodeManagerImpl();

        // set current role
        void SetRole(std::shared_ptr<CRole>& role);

        // send request to all
        void SendHeartToAll(HeartBeatResquest& request);
        void SendVoteToAll(VoteRequest& request);

        // get node numbers
        uint32_t GetNodeCount();
        // get all node info
        const std::unordered_map<std::string, std::shared_ptr<CNode>>& GetAllNode();
        // connect to all
        void ConnectToAll(const std::string& net_handle_list);
        // connect to a node
        void ConnectTo(const std::string& ip, uint16_t port);

        // new connect call back
        void NewConnectCallBack(const std::string& net_handle);
        // disconnect call back
        void DisConnectCallBack(const std::string& net_handle);
        // heart request call back
        void HeartRequestRecvCallBack(const std::string& net_handle, HeartBeatResquest& request);
        // heart response call back
        void HeartResponseRecvCallBack(const std::string& net_handle, HeartBeatResponse& response);
        // vote request call back
        void VoteRequestRecvCallBack(const std::string& net_handle, VoteRequest& request);
        // vote response call back
        void VoteResponseRecvCallBack(const std::string& net_handle, VoteResponse& response);
        // node find about
        void NodeInfoRequestCallBack(const std::string& net_handle, NodeInfoRequest& request);
        void NodeInfoResponseCallBack(const std::string& net_handle, NodeInfoResponse& response);
    private:
        // get a node
        std::shared_ptr<CNode> GetNode(const std::string& net_handle);
    private:
        // all node
        std::unordered_map<std::string, std::shared_ptr<CNode>> _node_map;
        // current connection net handle to remote net handle
        std::unordered_map<std::string, std::string>            _mapping_handle;
        // currnet role ptr
        std::shared_ptr<CRole> _current_role;
        // net 
        std::shared_ptr<CNet>  _net;
        // current net handle
        std::string            _cur_net_handle;
    };
}

#endif