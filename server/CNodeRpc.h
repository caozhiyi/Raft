#ifndef HADMAP_RPC_SERVICE_H
#define HADMAP_RPC_SERVICE_H

#include "raft_rpc.pb.h"

namespace raft {

    class CMsgRouter;
	class CNodeRpc : public RaftService {
	public:
        CNodeRpc(CMsgRouter* router);
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

        virtual void client_msg(::google::protobuf::RpcController* controller,
            const ::raft::ClientRequest* request,
            ::raft::ClientResponse* response,
            ::google::protobuf::Closure* done);

        virtual void rpc_hello(::google::protobuf::RpcController* controller,
            const ::raft::HelloResquest* request,
            ::raft::HelloResponse* response,
            ::google::protobuf::Closure* done);
        
    private:
        CMsgRouter* _msg_router;
	}; 
}
#endif