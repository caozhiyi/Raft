#ifndef RAFT_INTERFACE_NET
#define RAFT_INTERFACE_NET

#include <memory>
#include <functional>

#include "message.pb.h"

namespace raft {

    class CNode;
    class CNodeManager;
    class CNet {
    public:
        CNet(std::shared_ptr<CNodeManager> node_manager) : _node_manager(node_manager) {}
        virtual ~CNet() {}

        // start to listen
        virtual void Init(uint16_t thread_num) = 0;
        virtual bool Start(const std::string& ip, uint16_t port) = 0;
        virtual void Join() = 0;
        virtual void Dealloc() = 0;

        // connect to
        virtual void ConnectTo(const std::string& ip, uint16_t port) = 0;
        virtual void DisConnect(const std::string& net_handle) = 0;

        // node info
        virtual void SendNodeInfoRequest(const std::string& net_handle, NodeInfoRequest& request) = 0;
        virtual void SendNodeInfoResponse(const std::string& net_handle, NodeInfoResponse& response) = 0;
        // heart beat
        virtual void SendHeartRequest(const std::string& net_handle, HeartBeatResquest& request) = 0;
        virtual void SendHeartResponse(const std::string& net_handle, HeartBeatResponse& response) = 0;
        // vote
        virtual void SendVoteRequest(const std::string& net_handle, VoteRequest& request) = 0;
        virtual void SendVoteResponse(const std::string& net_handle, VoteResponse& response) = 0;
        // client about
        virtual void SendEntriesRequest(const std::string& net_handle, EntriesRequest& request) = 0;
        virtual void SendEntriesResponse(const std::string& net_handle, EntriesResponse& response) = 0;
    protected:
        std::shared_ptr<CNodeManager> _node_manager;
    };
}

#endif