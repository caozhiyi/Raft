#ifndef HADMAP_RPC_SERVICE_H
#define HADMAP_RPC_SERVICE_H

#include "raft_rpc.pb.h"

class CNode;
namespace raft {

	class CNodeRpc : public RaftService
	{
	public:
        CNodeRpc(CNode _node;);
		virtual ~CNodeRpc();

        virtual void RpcNodeHeart(::google::protobuf::RpcController* controller,
            const ::raft_rpc::HeartRequest* request,
            ::raft_rpc::HeartResponse* response,
            ::google::protobuf::Closure* done);
        virtual void RpcNodeVote(::google::protobuf::RpcController* controller,
            const ::raft_rpc::VoteResuest* request,
            ::raft_rpc::VoteToResponse* response,
            ::google::protobuf::Closure* done);
        
    private:
        CNode* _node;
	}; 
}
#endif