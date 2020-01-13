#ifndef RAFT_INTERFACE_NODEMANAGER
#define RAFT_INTERFACE_NODEMANAGER

#include "message.pb.h"

namespace raft {

    class CRole;
    class CNodeManager {
    public:
        CNodeManager() {}
        virtual ~CNodeManager() {}
        // set current role
        virtual void SetRole(std::shared_ptr<CRole>& role) = 0;
        // send request to all
        virtual void SendHeartToAll(HeartBeatResquest& request) = 0;
        virtual void SendVoteToAll(VoteRequest& request) = 0;

        // get node numbers
        virtual uint32_t GetNodeCount() = 0;
        // get all node info
        virtual const std::unordered_map<std::string, std::shared_ptr<CNode>>& GetAllNode() = 0;

        // connect to all
        virtual void ConnectToAll(const std::string& net_handle_list) = 0;
        // connect to a node
        virtual void ConnectTo(const std::string& ip, uint16_t port) = 0;

        // new connect call back
        virtual void NewConnectCallBack(const std::string& net_handle) = 0;
        // disconnect call back
        virtual void DisConnectCallBack(const std::string& net_handle) = 0;
        // heart request call back
        virtual void HeartRequestRecvCallBack(const std::string& net_handle, HeartBeatResquest& request) = 0;
        // heart response call back
        virtual void HeartResponseRecvCallBack(const std::string& net_handle, HeartBeatResponse& response) = 0;
        // vote request call back
        virtual void VoteRequestRecvCallBack(const std::string& net_handle, VoteRequest& request) = 0;
        // vote response call back
        virtual void VoteResponseRecvCallBack(const std::string& net_handle, VoteResponse& response) = 0;
        
        // node find about
        virtual void NodeInfoRequestCallBack(const std::string& net_handle, NodeInfoRequest& request) = 0;
        virtual void NodeInfoResponseCallBack(const std::string& net_handle, NodeInfoResponse& response) = 0;
    };
}

#endif