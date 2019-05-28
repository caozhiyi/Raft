#ifndef HADMAP_RPC_SERVICE_H
#define HADMAP_RPC_SERVICE_H

#include "raft_rpc.pb.h"
#include "CNode.h"

namespace raft {

	class CNodeRpc : public RaftService {
	public:
        CNodeRpc(CNode* node);
		virtual ~CNodeRpc();

        virtual void rpc_heart(::google::protobuf::RpcController* controller,
            const ::raft::HeartRequest* request,
            ::raft::HeartResponse* response,
            ::google::protobuf::Closure* done);

        virtual void rpc_vote(::google::protobuf::RpcController* controller,
            const ::raft::VoteResuest* request,
            ::raft::VoteResponse* response,
            ::google::protobuf::Closure* done);

        virtual void rpc_node_info(::google::protobuf::RpcController* controller,
            const ::raft::NodeInfoRequest* request,
            ::raft::NodeInfoResponse* response,
            ::google::protobuf::Closure* done);
        
    private:
        CNode* _node;
	}; 
}
#endif