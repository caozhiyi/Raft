#ifndef RAFT_RAFT_RAFTNODEIMPL
#define RAFT_RAFT_RAFTNODEIMPL

#include "INode.h"

namespace raft {

    class CNet;
    class NodeImpl: public CNode
    {
    public:
        NodeImpl(std::shared_ptr<CNet>& net, const std::string& handle);
        ~NodeImpl();
        // net handle
        void SetNetHandle(const std::string& handle);
        const std::string& GetNetHandle();
        // get send to the node next entries index
        uint64_t GetNextIndex();
        void SetNextIndex(uint64_t index);
        // get the node newest entries index
        uint64_t GetNewestIndex();
        void SetNewestIndex(uint64_t index);

        // heart beat
        void SendHeartRequest(HeartBeatResquest& request);
        void SendHeartResponse(HeartBeatResponse& response);
        // vote
        void SendVoteRequest(VoteRequest& request);
        void SendVoteResponse(VoteResponse& response);
    private:
        std::string  _net_handle;       // ip + port
        uint64_t     _sended_newest_index;
        uint64_t     _should_send_index;
        std::shared_ptr<CNet> _net;
    };
}

#endif