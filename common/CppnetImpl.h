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
        vote_reponse        = 4
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
    class CCppNet {
    public:
        CCppNet();
        ~CCppNet();
        // start to listen
        bool Start(const std::string& ip, uint16_t port);
        void Join();
        // heart beat
        void SendHeartRequest(const std::string& net_handle, HeartBeatResquest& request);
        void SendHeartResponse(const std::string& net_handle, HeartBeatResponse& response);
        // vote
        void SendVoteRequest(const std::string& net_handle, VoteRequest& request);
        void SendVoteResponse(const std::string& net_handle, VoteResponse& response);

        // set new connect call back
        void SetNewConnectCallBack(absl::FunctionRef<void(absl::string_view net_handle)> func);
        // set disconnect call back
        void SetDisConnectCallBack(absl::FunctionRef<void(absl::string_view net_handle)> func);
        // set heart request call back
        void SetHeartRequestRecvCallBack(absl::FunctionRef<void(absl::string_view, HeartBeatResquest&)> func);
        // set heart response call back
        void SetHeartResponseRecvCallBack(absl::FunctionRef<void(absl::string_view, HeartBeatResponse&)> func);
        // set vote request call back
        void SetVoteRequestRecvCallBack(absl::FunctionRef<void(absl::string_view, VoteRequest&)> func);
        // set vote response call back
        void SetVoteResponseRecvCallBack(absl::FunctionRef<void(absl::string_view, VoteResponse&)> func);
    private:
        // cppnet call back
        void Connected(const cppnet::Handle& handle, uint32_t err);
        void DisConnected(const cppnet::Handle& handle, uint32_t err);
        void Sended(const Handle& handle, uint32_t len, uint32_t err);
        void Recved((const Handle& handle, base::CBuffer* data, uint32_t len, uint32_t err);
        // bag about
        std::string BagToString(CppBag& bag);
        bool StringToBag(const std::string& data, std::vector<CppBag>& bag);
        // get net handle
        std::string GetNetHandle(const cppnet::Handle& handle);
        // send to net
        void SendToNet(const std::string& net_handle,std::string& data);
        // handle bag
        void HandleBag(const std::string& net_handle, const CppBag& bag);

    private:
        // net handle to cppnet handle
        std::map<std::string, cppnet::Handle>                                 _net_2_handle_map;
        std::map<cppnet::Handle, std::string>                                 _handle_2_net_map;

        // all call back
        std::function<void(void(absl::string_view, HeartBeatResquest&)>       _heart_request_call_back;
        std::function<void(absl::string_view, HeartBeatResponse&)>            _heart_response_call_back;
        std::function<void(absl::string_view, VoteRequest&)>                  _vote_request_call_back;
        std::function<void(absl::string_view, VoteResponse&)>                 _vote_response_call_back;
        std::function<void(absl::string_view)>                                _new_connect_call_back;
        std::function<void(absl::string_view)>                                _dis_connect_call_back;
    };
}

#endif