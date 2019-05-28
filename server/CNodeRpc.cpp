#include <brpc/server.h>
#include <json2pb/pb_to_json.h>

#include "CNodeRpc.h"
#include "common.h"
#include "CNode.h"
#include "Log.h"

using namespace raft;

CNodeRpc::CNodeRpc(CNode* node) : _node(node) {

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
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
    std::string ip = butil::endpoint2str(cntl->remote_side()).c_str();
    
    std::vector<std::string> msg_vec;
    for (int i = 0; i < request->msg_size(); i++) {
        msg_vec.push_back(request->msg(i));
    }
    long long version = request->version();
    bool done_msg = request->done_msg();
    long long new_version = 0;
    _node->HandleHeart(ip, msg_vec, version, done_msg, new_version);
    
    response->set_version(new_version);

    ret = json2pb::ProtoMessageToJson(*response, &json, options, &error);
    if (ret) {
        LOG_DEBUG("Received rpc_heart. response : %s", json.c_str());;
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
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
    std::string ip = butil::endpoint2str(cntl->remote_side()).c_str();
    long long version = request->version();
    
    bool vote = false;
    _node->HandleVote(ip, version, vote);

    response->set_vote(vote);

    ret = json2pb::ProtoMessageToJson(*response, &json, options, &error);
    if (ret) {
        LOG_DEBUG("Received rpc_vote. response : %s", json.c_str());;
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
    brpc::Controller* cntl = static_cast<brpc::Controller*>(controller);
    std::string ip = butil::endpoint2str(cntl->remote_side()).c_str();

    std::vector<std::string> info_vec;
    for (int i = 0; i < request->ip_port_size(); i++) {
        info_vec.push_back(request->ip_port(i));
    }

    if (info_vec.empty()) {
        _node->GetNodeInfo(ip, info_vec);
        for (auto iter = info_vec.begin(); iter != info_vec.end(); ++iter) {
            response->add_ip_port(*iter);
        }

    } else {
        _node->AddNewNode(ip, info_vec);
    }

    ret = json2pb::ProtoMessageToJson(*response, &json, options, &error);
    if (ret) {
        LOG_DEBUG("Received rpc_node_info. response : %s", json.c_str());;
    }
}