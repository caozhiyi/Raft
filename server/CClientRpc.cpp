#include <brpc/server.h>
#include <json2pb/pb_to_json.h>

#include "CClientRpc.h"
#include "common.h"
#include "CNode.h"
#include "client_rpc.pb.h"
#include "Log.h"

using namespace raft;

CClientRpc::CClientRpc(CListener* listener) : _listener(listener) {
    
}

CClientRpc::~CClientRpc() {

}

void CClientRpc::client_msg(::google::protobuf::RpcController* controller,
    const ::raft::ClientRequest* request,
    ::raft::ClientResponse* response,
    ::google::protobuf::Closure* done) {

    json2pb::Pb2JsonOptions options;
    options.bytes_to_base64 = true;
    options.pretty_json = true;
    std::string json;
    std::string error;
    bool ret = json2pb::ProtoMessageToJson(*request, &json, options, &error);
    if (ret) {
        LOG_DEBUG("Received client_msg. request : %s", json.c_str());;
    }

    // get client ip info
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
    std::string ip = butil::endpoint2str(cntl->remote_side()).c_str();
    std::string msg = request->msg();

    _listener->HandleClient(ip, msg, response, done);
}