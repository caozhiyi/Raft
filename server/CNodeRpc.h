#ifndef HADMAP_RPC_SERVICE_H
#define HADMAP_RPC_SERVICE_H

#include "raft_rpc.pb.h"

class CNode;
namespace raft {

	class CNodeRpc : public RaftService
	{
	public:
        CNodeRpc(CNode* node);
		virtual ~CNodeRpc();

        virtual void rpc_heart(::google::protobuf::RpcController* controller,
            const ::raft_rpc::HeartRequest* request,
            ::raft_rpc::HeartResponse* response,
            ::google::protobuf::Closure* done);

        virtual void rpc_vote(::google::protobuf::RpcController* controller,
            const ::raft_rpc::VoteResuest* request,
            ::raft_rpc::VoteToResponse* response,
            ::google::protobuf::Closure* done);

        virtual void rpc_node_info(::google::protobuf::RpcController* controller,
            const ::raft_rpc::NodeInfoRequest* request,
            ::raft_rpc::NodeInfoResponse* response,
            ::google::protobuf::Closure* done);
        
    private:
        CNode* _node;
	}; 
}
#endif