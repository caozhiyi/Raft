#ifndef RAFT_INTERFACE_NODE
#define RAFT_INTERFACE_NODE

#include "message.pb.h"
#include "absl/functional/function_ref.h"

namespace raft {

    class CNode {
    public:
        CNode() {}
        virtual ~CNode() {}
        // net handle
        virtual void SetNetHandle(const std::string& handle) = 0;
        virtual const std::string& GetNetHandle() = 0;

        // get send to the node next entries index
        virtual uint64_t GetNextIndex() = 0;
        virtual void SetNextIndex(uint64_t index) = 0;
        // get the node newest entries index
        virtual uint64_t GetMatchIndex() = 0;
        virtual void SetMatchIndex(uint64_t index) = 0;

        // node info
        virtual void SendNodeInfoRequest(NodeInfoRequest& request) = 0;
        virtual void SendNodeInfoResponse(NodeInfoResponse& response) = 0;
        // heart beat
        virtual void SendHeartRequest(HeartBeatResquest& request) = 0;
        virtual void SendHeartResponse(HeartBeatResponse& response) = 0;
        // vote
        virtual void SendVoteRequest(VoteRequest& request) = 0;
        virtual void SendVoteResponse(VoteResponse& response) = 0;
    };
}

#endif