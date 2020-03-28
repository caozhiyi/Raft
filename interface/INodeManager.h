#ifndef RAFT_INTERFACE_NODEMANAGER
#define RAFT_INTERFACE_NODEMANAGER

#include "message.pb.h"

namespace raft {

    class CNet;
    class CRole;
    class CNodeManager {
    public:
        CNodeManager() {}
        virtual ~CNodeManager() {}
        // set net impl point
        virtual void SetNet(std::shared_ptr<CNet> net) = 0;
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

        // connection call back
        virtual void NewConnectCallBack(const std::string& net_handle) = 0;
        virtual void DisConnectCallBack(const std::string& net_handle) = 0;
        // heart call back
        virtual void HeartRequestRecvCallBack(const std::string& net_handle, HeartBeatResquest& request) = 0;
        virtual void HeartResponseRecvCallBack(const std::string& net_handle, HeartBeatResponse& response) = 0;
        // vote call back
        virtual void VoteRequestRecvCallBack(const std::string& net_handle, VoteRequest& request) = 0;
        virtual void VoteResponseRecvCallBack(const std::string& net_handle, VoteResponse& response) = 0;
        // node find about
        virtual void NodeInfoRequestCallBack(const std::string& net_handle, NodeInfoRequest& request) = 0;
        virtual void NodeInfoResponseCallBack(const std::string& net_handle, NodeInfoResponse& response) = 0;
        // entries call back
        virtual void EntriesRequestCallBack(const std::string& net_handle, EntriesRequest& request) = 0;
        virtual void EntriesResponseCallBack(const std::string& net_handle, EntriesResponse& response) = 0;
    };
}

#endif