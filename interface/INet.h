#ifndef RAFT_INTERFACE_NET
#define RAFT_INTERFACE_NET

#include "message.pb.h"
#include "absl/strings/string_view.h"
#include "absl/functional/function_ref.h"

namespace raft {

    class CNode;
    class CNet {
    public:
        CNet() {}
        virtual ~CNet() {}
        // start to listen
        virtual bool Start(absl::string_view ip, uint16_t port);
        // heart beat
        virtual void SendHeartRequest(absl::string_view ip, uint16_t port, HeartBeatResquest& request) = 0;
        virtual void SendHeartResponse(absl::string_view ip, uint16_t port, HeartBeatResponse& response) = 0;
        // vote
        virtual void SendVoteRequest(absl::string_view ip, uint16_t port, VoteResquest& request) = 0;
        virtual void SendVoteResponse(absl::string_view ip, uint16_t port, VoteResponse& response) = 0;

        // set new connect call back
        virtual void SetNewConnectCallBack(absl::FunctionRef<void(CNode&)> func) = 0;
        // set disconnect call back
        virtual void SetDisConnectCallBack(absl::FunctionRef<void(absl::string_view ip, uint16_t port)> func) = 0;
        // set heart request call back
        virtual void SetHeartRequestRecvCallBack(absl::FunctionRef<void(absl::string_view, uint16_t, HeartBeatResquest&)> func) = 0;
        // set heart response call back
        virtual void SetHeartResponseRecvCallBack(absl::FunctionRef<void(absl::string_view, uint16_t, HeartBeatResponse&)> func) = 0;
        // set vote request call back
        virtual void SetVoteRequestRecvCallBack(absl::FunctionRef<void(absl::string_view, uint16_t, VoteResquest&)> func) = 0;
        // set vote response call back
        virtual void SetVoteResponseRecvCallBack(absl::FunctionRef<void(absl::string_view, uint16_t, VoteResponse&)> func) = 0;
        
    };
}

#endif