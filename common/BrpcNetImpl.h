#ifndef RAFT_CPMMON_BRPCNETIMPL
#define RAFT_CPMMON_BRPCNETIMPL

#include <map>
#include <queue>
#include <functional>

#include "INet.h"
#include "NetCommon.h"
#include "message.pb.h"
#include "brpc/server.h"
#include "brpc/channel.h"

namespace raft {
    
    class CNode;
    class CBrpcNetImpl : public CNet, public RaftService {
    public:
        CBrpcNetImpl(std::shared_ptr<CNodeManager> node_manager);
        ~CBrpcNetImpl();

        void Init(uint16_t thread_num);
        // start to listen
        bool Start(const std::string& ip, uint16_t port);
        void Join();
        void Dealloc();
        // connect to
        void ConnectTo(const std::string& ip, uint16_t port);
        void DisConnect(const std::string& net_handle);

        // node info
        void SendNodeInfoRequest(const std::string& net_handle, NodeInfoRequest& request);
        void SendNodeInfoResponse(const std::string& net_handle, NodeInfoResponse& response);
        // heart beat
        void SendHeartRequest(const std::string& net_handle, HeartBeatResquest& request);
        void SendHeartResponse(const std::string& net_handle, HeartBeatResponse& response);
        // vote
        void SendVoteRequest(const std::string& net_handle, VoteRequest& request);
        void SendVoteResponse(const std::string& net_handle, VoteResponse& response);

        // client about
        void SendEntriesRequest(const std::string& net_handle, EntriesRequest& request);
        void SendEntriesResponse(const std::string& net_handle, EntriesResponse& response);

    private:
        void RpcHeart(PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::raft::HeartBeatResquest* request,
                       ::raft::HeartBeatResponse* response,
                       ::google::protobuf::Closure* done);
        void RpcVote(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::raft::VoteRequest* request,
                       ::raft::VoteResponse* response,
                       ::google::protobuf::Closure* done);
        void RpcNodeInfo(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::raft::NodeInfoRequest* request,
                       ::raft::NodeInfoResponse* response,
                       ::google::protobuf::Closure* done);
        void RpcEntries(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                       const ::raft::EntriesRequest* request,
                       ::raft::EntriesResponse* response,
                       ::google::protobuf::Closure* done);

    private:
        struct RpcDoneParam {
            std::string _net_handle;
            void*       _response;
            MessageType _type;
            RpcDoneParam(std::string net_handle, void* response, MessageType type) :
                _net_handle(net_handle), _response(response), _type(type) {}
            ~RpcDoneParam() {
                delete _response;
            }
        };

        // rpc done call back
        void RpcDone(void* param, brpc::Controller* cntl);
        void ConnectTo(const std::string& ip_port);

        // add response to cache queue
        void CacheResponse(const std::string& net_handle, ::google::protobuf::Closure* done, void* response);

    private:
        brpc::Server  _server;

        struct ChannelInfo {
            raft::RaftService_Stub* _stub;
            std::queue<std::pair<::google::protobuf::Closure*, void*>> _closure_response_queue;
            ChannelInfo(raft::RaftService_Stub* stub) : _stub(stub) {}
            ChannelInfo() : _stub(nullptr) {}
            ~ChannelInfo() {
                if (_stub) {
                    delete _stub->channel();
                    delete _stub;
                }
                _closure_response_queue.clear();
            }
        };

        std::map<std::string, ChannelInfo> _channel_map;
    };
}

#endif