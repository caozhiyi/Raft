#ifndef RAFT_INTERFACE_NODEMANAGER
#define RAFT_INTERFACE_NODEMANAGER

#include "message.pb.h"
#include "absl/functional/function_ref.h"

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
    };
}

#endif