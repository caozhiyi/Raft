#ifndef HADMAP_RPC_SERVICE_H
#define HADMAP_RPC_SERVICE_H

#include "raft_rpc.pb.h"

class CListener;
namespace raft {

	class CClientRpc : public ClientService
	{
	public:
        CClientRpc(CListener* listener);
		virtual ~CClientRpc();

        virtual void client_msg(::google::protobuf::RpcController* controller,
            const ::raft_rpc::ClientRequest* request,
            ::raft_rpc::ClientResponse* response,
            ::google::protobuf::Closure* done);

    private:
        CListener* _listener;
	}; 
}
#endif