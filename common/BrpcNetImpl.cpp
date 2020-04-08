#include "Log.h"
#include "Socket.h"
#include "BrpcNetImpl.h"
#include "INodeManager.h"
#include "absl/strings/numbers.h"

using namespace raft;

static const std::string __server_version = "raft_server_1.0";
static const int         __rpc_time_out = 200; // 200ms

CBrpcNetImpl::CBrpcNetImpl(std::shared_ptr<CNodeManager> node_manager) : CNet(node_manager) {

}

CBrpcNetImpl::~CBrpcNetImpl() {

}

void CBrpcNetImpl::Init(uint16_t thread_num) {
    _server.set_version(__server_version);

    if (_server.AddService(this, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        base::LOG_ERROR("fail to add service.");
    }
}

bool CBrpcNetImpl::Start(const std::string& ip, uint16_t port) {
    brpc::ServerOptions options;
    std::string ip_port = ip + ":" + std::to_string(port);
    if (_server.Start(ip_port.c_str(), &options) != 0) {
        LOG_ERROR("Fail to start server. ip : %s", _local_ip_port.c_str());
        return false;
    }
    return true;
}

void CBrpcNetImpl::Join() {
    _server.Join();
}

void CBrpcNetImpl::Dealloc() {
    _server.Stop();
}

void CBrpcNetImpl::ConnectTo(const std::string& ip, uint16_t port) {
    std::string ip_port = ip + ":" + std::to_string(port);
    ConnectTo(ip_port);
}

void CBrpcNetImpl::DisConnect(const std::string& net_handle) {
    _net_2_handle_map.erase(net_handle);
}

void CBrpcNetImpl::SendNodeInfoRequest(const std::string& net_handle, NodeInfoRequest& request) {
    if (_channel_map.count(net_handle) < 1) {
        ConnectTo(net_handle);
    }
    auto iter = _channel_map.find(net_handle);
    NodeInfoResponse* response = new NodeInfoResponse;
    brpc::Controller* cntl = new brpc::Controller;
    RpcDoneParam param = new RpcDoneParam(net_handle, (void*)response, node_info_response);
    cntl->set_timeout_ms(__rpc_time_out);
    auto closure = google::protobuf::NewCallback(&CBrpcNetImpl::RpcDone, (void*)param, cntl);
    iter->second._stub->RpcNodeInfo(cntl, &request, response, closure);
}

void CBrpcNetImpl::SendNodeInfoResponse(const std::string& net_handle, NodeInfoResponse& response) {
    auto iter = _channel_map.find(net_handle);
    if (iter == _net_2_handle_map.end()) {
        return;
    }
    auto& channel_info = iter->second;
    auto closure_and_response = channel_info._closure_response_queue.front();
    channel_info._closure_response_queue.pop();

    ((NodeInfoResponse*)closure_and_response.second)->CopyFrom(response);
    closure_and_response.first->Run();
}

void CBrpcNetImpl::SendHeartRequest(const std::string& net_handle, HeartBeatResquest& request) {
    if (_channel_map.count(net_handle) < 1) {
        ConnectTo(net_handle);
    }
    auto iter = _channel_map.find(net_handle);
    HeartBeatResponse* response = new HeartBeatResponse;
    brpc::Controller* cntl = new brpc::Controller;
    RpcDoneParam param = new RpcDoneParam(net_handle, (void*)response, heart_beat_response);
    cntl->set_timeout_ms(__rpc_time_out);
    auto closure = google::protobuf::NewCallback(&CBrpcNetImpl::RpcDone, (void*)param, cntl);
    iter->second._stub->RpcHeart(cntl, &request, response, closure);
}

void CBrpcNetImpl::SendHeartResponse(const std::string& net_handle, HeartBeatResponse& response) {
    if (_channel_map.count(net_handle) < 1) {
        ConnectTo(net_handle);
    }
    auto iter = _channel_map.find(net_handle);

    auto& channel_info = iter->second;
    auto closure_and_response = channel_info._closure_response_queue.front();
    channel_info._closure_response_queue.pop();

    ((HeartBeatResponse*)closure_and_response.second)->CopyFrom(response);
    closure_and_response.first->Run();
}

void CBrpcNetImpl::SendVoteRequest(const std::string& net_handle, VoteRequest& request) {
    if (_channel_map.count(net_handle) < 1) {
        ConnectTo(net_handle);
    }
    auto iter = _channel_map.find(net_handle);
    
    VoteResponse* response = new VoteResponse;
    brpc::Controller* cntl = new brpc::Controller;
    RpcDoneParam param = new RpcDoneParam(net_handle, (void*)response, vote_response);
    cntl->set_timeout_ms(__rpc_time_out);
    auto closure = google::protobuf::NewCallback(&CBrpcNetImpl::RpcDone, (void*)param, cntl);
    iter->second._stub->RpcVote(cntl, &request, response, closure);
}

void CBrpcNetImpl::SendVoteResponse(const std::string& net_handle, VoteResponse& response) {
    auto iter = _channel_map.find(net_handle);
    if (iter == _channel_map.end()) {
        return;
    }
    auto& channel_info = iter->second;
    auto closure_and_response = channel_info._closure_response_queue.front();
    channel_info._closure_response_queue.pop();

    ((VoteResponse*)closure_and_response.second)->CopyFrom(response);
    closure_and_response.first->Run();
}

void CBrpcNetImpl::SendEntriesRequest(const std::string& net_handle, EntriesRequest& request) {
    if (_channel_map.count(net_handle) < 1) {
        ConnectTo(net_handle);
    }
    auto iter = _channel_map.find(net_handle);
  
    EntriesResponse* response = new EntriesResponse;
    brpc::Controller* cntl = new brpc::Controller;
    RpcDoneParam param = new RpcDoneParam(net_handle, (void*)response, entries_response);
    cntl->set_timeout_ms(__rpc_time_out);
    auto closure = google::protobuf::NewCallback(&CBrpcNetImpl::RpcDone, (void*)param, cntl);
    iter->second._stub->RpcEntries(cntl, &request, response, closure);
}

void CBrpcNetImpl::SendEntriesResponse(const std::string& net_handle, EntriesResponse& response) {
    auto iter = _channel_map.find(net_handle);
    if (iter == _channel_map.end()) {
        return;
    }
    auto& channel_info = iter->second;
    auto closure_and_response = channel_info._closure_response_queue.front();
    channel_info._closure_response_queue.pop();

    ((EntriesResponse*)closure_and_response.second)->CopyFrom(response);
    closure_and_response.first->Run();
}

void CBrpcNetImpl::RpcHeart(PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::raft::HeartBeatResquest* request,
                       ::raft::HeartBeatResponse* response,
                       ::google::protobuf::Closure* done) {
    std::string net_handle = request->local_ip();
    CacheResponse(net_handle, done, (void*)response);
    _node_manager->HeartRequestRecvCallBack(net_handle, *request);
}

void CBrpcNetImpl::RpcVote(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::raft::VoteRequest* request,
                       ::raft::VoteResponse* response,
                       ::google::protobuf::Closure* done) {
    std::string net_handle = request->local_ip();
    CacheResponse(net_handle, done, (void*)response);
    _node_manager->VoteRequestRecvCallBack(net_handle, *request);
}

void CBrpcNetImpl::RpcNodeInfo(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::raft::NodeInfoRequest* request,
                       ::raft::NodeInfoResponse* response,
                       ::google::protobuf::Closure* done) {
    std::string net_handle = request->local_ip();
    CacheResponse(net_handle, done, (void*)response);
    _node_manager->NodeInfoRequestRecvCallBack(net_handle, *request);
}

void CBrpcNetImpl::RpcEntries(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::raft::EntriesRequest* request,
                       ::raft::EntriesResponse* response,
                       ::google::protobuf::Closure* done) {
    std::string net_handle = request->local_ip();
    CacheResponse(net_handle, done, (void*)response);
    _node_manager->EntriesRequestRecvCallBack(net_handle, *request);
}

void CBrpcNetImpl::RpcDone(void* param, brpc::Controller* cntl) {
    auto response = ((RpcDoneParam*)param)->_response;
    auto type = ((RpcDoneParam*)param)->_type;
    auto net_handle = std::move((RpcDoneParam*)param)->_net_handle);

    std::unique_ptr<raft::ClientResponse> response_guard(response);
    std::unique_ptr<brpc::Controller> cntl_guard(cntl);
    if (cntl->Failed()) {
        LOG_ERROR("rpc call server failed. err info : %s", cntl->ErrorText().c_str());
        delete ((RpcDoneParam*)param);
        return;
    }

    switch (bag._header._field._type)
    {
    case heart_beat_response:
        {
            _node_manager->HeartResponseRecvCallBack(net_handle, *((HeartBeatResponse*)response));
            break;
        }
    case vote_response:
        {
            _node_manager->VoteResponseRecvCallBack(net_handle, *((VoteResponse*)response));
            break;
        }
    case entries_response:
        {
            _node_manager->EntriesResponseCallBack(net_handle, *((EntriesResponse*)response));
            break;
        }
    case node_info_response:
        {
            _node_manager->NodeInfoResponseCallBack(net_handle, *((NodeInfoResponse*)response));
            break;
        }
    default:
        base::LOG_ERROR("unknow type while handle bag");
        break;
    }
    delete ((RpcDoneParam*)param);
}

void CBrpcNetImpl::ConnectTo(const std::string& ip_port) {
    brpc::Channel *channel = new brpc::Channel;
    brpc::ChannelOptions options;
    channel->Init(ip_port.c_str(), &options);
    raft::RaftService_Stub *stub = new raft::RaftService_Stub(channel);
    _channel_map[ip_port] = ChannelInfo(stub);
}

void CBrpcNetImpl::CacheResponse(const std::string& net_handle, ::google::protobuf::Closure* done, void* response) {
    if (_channel_map.count(net_handle) < 1) {
        ConnectTo(net_handle);
    }
    _channel_map[net_handle]._closure_response_queue.push(std::make_pair(done, (void*)response));
}