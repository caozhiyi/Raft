#include <brpc/server.h>
#include <butil/logging.h>

#include "CClientRpc.h"
#include "common.h"
#include "CNode.h"
#include "client_rpc.pb.h"

using namespace raft;

CClientRpc::CClientRpc(CListener* listener) : _listener(listener) {
    
}

CClientRpc::~CClientRpc() {

}

void CClientRpc::client_msg(::google::protobuf::RpcController* controller,
    const ::raft::ClientRequest* request,
    ::raft::ClientResponse* response,
    ::google::protobuf::Closure* done) {
    
}