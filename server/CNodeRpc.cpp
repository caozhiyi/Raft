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

void CNodeRpc::rpc_heart(::google::protobuf::RpcController* controller,
    const ::raft_rpc::HeartRequest* request,
    ::raft_rpc::HeartResponse* response,
    ::google::protobuf::Closure* done) {
    
    baidu::rpc::ClosureGuard done_guard(done);

    // get client ip info
    baidu::rpc::Controller* cntl = static_cast<baidu::rpc::Controller*>(controller);
    std::string ip = base::endpoint2str(cntl->remote_side()).c_str();
    
    std::vector<std::string> msg_vec;
    for (int i = 0; i < request->msg_size(); i++) {
        msg_vec.push_back(request->msg(i));
    }
    long long version = request->version();
    bool done_msg = request->done_msg();
    long long new_version = 0;
    _node->HandleHeart(ip, msg_vec, version, done_msg, new_version);
    
    response->set_version(new_version);
}

void CNodeRpc::rpc_vote(::google::protobuf::RpcController* controller,
    const ::raft_rpc::VoteResuest* request,
    ::raft_rpc::VoteToResponse* response,
    ::google::protobuf::Closure* done) {

    baidu::rpc::ClosureGuard done_guard(done);

    // get client ip info
    baidu::rpc::Controller* cntl = static_cast<baidu::rpc::Controller*>(controller);
    std::string ip = base::endpoint2str(cntl->remote_side()).c_str();
    
    bool vote = false;
    _node->HandleVote(ip, vote);

    response->set_vote(vote);
}

void CNodeRpc::rpc_node_info(::google::protobuf::RpcController* controller,
    const ::raft_rpc::NodeInfoRequest* request,
    ::raft_rpc::NodeInfoResponse* response,
    ::google::protobuf::Closure* done) {
    
    baidu::rpc::ClosureGuard done_guard(done);

    // get client ip info
    baidu::rpc::Controller* cntl = static_cast<baidu::rpc::Controller*>(controller);
    std::string ip = base::endpoint2str(cntl->remote_side()).c_str();

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
}