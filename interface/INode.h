#ifndef RAFT_INTERFACE_NODE
#define RAFT_INTERFACE_NODE

#include "message.pb.h"
#include "absl/strings/string_view.h"
#include "absl/functional/function_ref.h"

namespace raft {

    class CNode {
    public:
        CNode() {}
        virtual ~CNode() {}
        // net handle
        virtual void SetNetHandle(absl::string_view handle) = 0;
        virtual std::string GetNetHandle() = 0;

        // get send to the node next entries index
        virtual uint64_t GetNextIndex() = 0;
        // get the node newest entries index
        virtual uint64_t GetNewestIndex() = 0;

        // heart beat
        virtual void SendHeartRequest(HeartBeatResquest& request) = 0;
        virtual void SendHeartResponse(HeartBeatResponse& response) = 0;
        // vote
        virtual void SendVoteRequest(VoteRequest& request) = 0;
        virtual void SendVoteResponse(VoteResponse& response) = 0;
    };
}

#endif