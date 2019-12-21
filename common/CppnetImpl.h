#ifndef RAFT_CPMMON_CPPNETIMPL
#define RAFT_CPMMON_CPPNETIMPL

#include <functional>
#include <map>
#include "INet.h"
#include "CppNet.h"
#include "CppDefine.h"

namespace raft {
    enum CppBagType {
        heart_beat_request  = 1,
        heart_beat_response = 2,
        vote_request        = 3,
        vote_reponse        = 4,
        client_type         = 5
    };

    enum ClientType {
        unknow_type         = 1,
        user_client         = 2,
        raft_node           = 3
    };

    static const uint32_t __header_len = sizeof(uint64_t);
    struct CppBag {
        union Header {
            struct Field {
                uint32_t  _len;
                uint32_t  _type;
            } _field;
            uint64_t      _data;
        } _header;
        std::string       _body;
    };

    class CNode;
    class CCppNet : public CNet {
    public:
        CCppNet();
        ~CCppNet();
        // start to listen
        bool Start(const std::string& ip, uint16_t port, uint16_t thread_num);
        void Join();
        void Dealloc();
        // connect to
        void ConnectTo(const std::string& ip, uint16_t port);
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
        void SendToClient(const std::string& net_handle, ClientResponse& response);
        // client call back
        void SetClientRecvCallBack(absl::FunctionRef<void(const std::string&, ClientRequest&)> func);
        void SetClientConnectCallBack(absl::FunctionRef<void(const std::string&)> func);
        void SetClientDisConnectCallBack(absl::FunctionRef<void(const std::string&)> func);

        // reft about
        // set new connect call back
        void SetNewConnectCallBack(absl::FunctionRef<void(const std::string&)> func);
        // set disconnect call back
        void SetDisConnectCallBack(absl::FunctionRef<void(const std::string&)> func);
        // set heart request call back
        void SetHeartRequestRecvCallBack(absl::FunctionRef<void(const std::string&, HeartBeatResquest&)> func);
        // set heart response call back
        void SetHeartResponseRecvCallBack(absl::FunctionRef<void(const std::string&, HeartBeatResponse&)> func);
        // set vote request call back
        void SetVoteRequestRecvCallBack(absl::FunctionRef<void(const std::string&, VoteRequest&)> func);
        // set vote response call back
        void SetVoteResponseRecvCallBack(absl::FunctionRef<void(const std::string&, VoteResponse&)> func);
    private:
        // cppnet call back
        void Connected(const cppnet::Handle& handle, uint32_t err);
        void DisConnected(const cppnet::Handle& handle, uint32_t err);
        void Sended(const cppnet::Handle& handle, uint32_t len, uint32_t err);
        void Recved(const cppnet::Handle& handle, base::CBuffer* data, uint32_t len, uint32_t err);
        // bag about
        std::string BagToString(CppBag& bag);
        bool StringToBag(const std::string& data, std::vector<CppBag>& bag_vec, uint32_t& used_size);
        // get net handle
        std::pair<std::string, ClientType> GetNetHandle(const cppnet::Handle& handle);
        // send to net
        void SendToNet(const std::string& net_handle, std::string& data, ClientType type = raft_node);
        // handle bag
        void HandleBag(const std::string& net_handle, const CppBag& bag);
        // check connect type
        void CheckConnectType(const cppnet::Handle& handle, const std::string& net_handle, CppBagType type);

    private:
        // net handle to cppnet handle
        std::map<std::string, cppnet::Handle>                                 _net_2_handle_map;
        std::map<cppnet::Handle, std::pair<std::string, ClientType>>          _handle_2_net_map;

        // raft call back
        std::function<void(const std::string&, HeartBeatResquest&)>           _heart_request_call_back;
        std::function<void(const std::string&, HeartBeatResponse&)>           _heart_response_call_back;
        std::function<void(const std::string&w, VoteRequest&)>                _vote_request_call_back;
        std::function<void(const std::string&, VoteResponse&)>                _vote_response_call_back;
        std::function<void(const std::string&)>                               _raft_connect_call_back;
        std::function<void(const std::string&)>                               _raft_dis_connect_call_back;
        // client about
        std::function<void(const std::string&, ClientRequest&)>               _client_recv_call_back;
        std::function<void(const std::string&)>                               _client_connect_call_back;
        std::function<void(const std::string&)>                               _client_dis_connect_call_back;
    };
}

#endif