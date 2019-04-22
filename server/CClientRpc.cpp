#include <baidu/rpc/server.h>
#include <base/logging.h>

#include "CNodeRpc.h"
#include "common.h"
#include "CNode.h"

using namespace raft;

CNodeRpc::CNodeRpc(CNode* node) : _node(node) {

}

CNodeRpc::~CNodeRpc() {

}

void CNodeRpc::client_msg(::google::protobuf::RpcController* controller,
    const ::raft_rpc::ClientRequest* request,
    ::raft_rpc::ClientResponse* response,
    ::google::protobuf::Closure* done) {
    
}