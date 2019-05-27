#ifndef HADMAP_RPC_CLIENT_H
#define HADMAP_RPC_CLIENT_H

#include "client_rpc.pb.h"
#include "CListener.h"

namespace raft {

    class CClientRpc : public ClientService {
    public:
        CClientRpc(CListener* listener);
        virtual ~CClientRpc();

        virtual void client_msg(::google::protobuf::RpcController* controller,
            const ::raft::ClientRequest* request,
            ::raft::ClientResponse* response,
            ::google::protobuf::Closure* done);

    private:
        CListener* _listener;
    };
}
#endif