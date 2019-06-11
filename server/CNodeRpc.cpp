#include <brpc/server.h>
#include <json2pb/pb_to_json.h>

#include "CNodeRpc.h"
#include "Log.h"
#include "CMsgRouter.h"

using namespace raft;

CNodeRpc::CNodeRpc(CMsgRouter* router) : _msg_router(router) {

}

CNodeRpc::~CNodeRpc() {

}

void CNodeRpc::rpc_heart(::google::protobuf::RpcController* controller,
    const ::raft::HeartRequest* request,
    ::raft::HeartResponse* response,
    ::google::protobuf::Closure* done) {
    
    brpc::ClosureGuard done_guard(done);

    json2pb::Pb2JsonOptions options;
    options.bytes_to_base64 = true;
    options.pretty_json = true;
    std::string json;
    std::string error;
    bool ret = json2pb::ProtoMessageToJson(*request, &json, options, &error);
    if (ret) {
        LOG_DEBUG("Received rpc_heart. request : %s", json.c_str());;
    }

    // get client ip info
    //brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
    //std::string ip = butil::endpoint2str(cntl->remote_side()).c_str();
    std::string ip = request->local_ip();

    std::vector<std::string> msg_vec;
    for (int i = 0; i < request->msg_size(); i++) {
        msg_vec.push_back(request->msg(i));
    }
    long long version = request->version();
    bool done_msg = request->done_msg();
    long long new_version = 0;
    _msg_router->HandleHeart(ip, msg_vec, version, done_msg, new_version);
    response->set_version(new_version);

    json.clear();
    ret = json2pb::ProtoMessageToJson(*response, &json, options, &error);
    if (ret) {
        LOG_DEBUG("Received rpc_heart. response : %s", json.c_str());
    }
}

void CNodeRpc::rpc_vote(::google::protobuf::RpcController* controller,
    const ::raft::VoteResuest* request,
    ::raft::VoteResponse* response,
    ::google::protobuf::Closure* done) {

    brpc::ClosureGuard done_guard(done);

    json2pb::Pb2JsonOptions options;
    options.bytes_to_base64 = true;
    options.pretty_json = true;
    std::string json;
    std::string error;
    bool ret = json2pb::ProtoMessageToJson(*request, &json, options, &error);
    if (ret) {
        LOG_DEBUG("Received rpc_vote. request : %s", json.c_str());;
    }

    // get client ip info
    //brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
    //std::string ip = butil::endpoint2str(cntl->remote_side()).c_str();
    std::string ip = request->local_ip();
    long long version = request->version();
    
    bool vote = false;
    _msg_router->HandleVote(ip, version, vote);
    response->set_vote(vote);

    json.clear();
    ret = json2pb::ProtoMessageToJson(*response, &json, options, &error);
    if (ret) {
        LOG_DEBUG("Received rpc_vote. response : %s", json.c_str());
    }
}

void CNodeRpc::rpc_node_info(::google::protobuf::RpcController* controller,
    const ::raft::NodeInfoRequest* request,
    ::raft::NodeInfoResponse* response,
    ::google::protobuf::Closure* done) {
    
    brpc::ClosureGuard done_guard(done);

    json2pb::Pb2JsonOptions options;
    options.bytes_to_base64 = true;
    options.pretty_json = true;
    std::string json;
    std::string error;
    bool ret = json2pb::ProtoMessageToJson(*request, &json, options, &error);
    if (ret) {
        LOG_DEBUG("Received rpc_node_info. request : %s", json.c_str());;
    }

    // get client ip info
    //brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
    //std::string ip = butil::endpoint2str(cntl->remote_side()).c_str();
    std::string ip = request->local_ip();

    std::vector<std::string> info_vec;
    for (int i = 0; i < request->ip_port_size(); i++) {
        info_vec.push_back(request->ip_port(i));
    }

    if (info_vec.empty()) {
        _msg_router->GetNodeInfo(ip, info_vec);
        for (auto iter = info_vec.begin(); iter != info_vec.end(); ++iter) {
            response->add_ip_port(*iter);
        }

    } else {
        _msg_router->AddNewNode(ip, info_vec);
    }

    json.clear();
    ret = json2pb::ProtoMessageToJson(*response, &json, options, &error);
    if (ret) {
        LOG_DEBUG("Received rpc_node_info. response : %s", json.c_str());
    }
}

void CNodeRpc::client_msg(::google::protobuf::RpcController* controller,
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

    _msg_router->HandleClient(ip, msg, response, done);
}

void CNodeRpc::rpc_hello(::google::protobuf::RpcController* controller,
    const ::raft::HelloResquest* request,
    ::raft::HelloResponse* response,
    ::google::protobuf::Closure* done) {

    brpc::ClosureGuard done_guard(done);
    json2pb::Pb2JsonOptions options;
    options.bytes_to_base64 = true;
    options.pretty_json = true;
    std::string json;
    std::string error;
    bool ret = json2pb::ProtoMessageToJson(*request, &json, options, &error);
    if (ret) {
        LOG_DEBUG("Received client_msg. request : %s", json.c_str());
    }

    //do nothing
}