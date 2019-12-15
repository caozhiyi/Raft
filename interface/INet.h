#ifndef RAFT_INTERFACE_NET
#define RAFT_INTERFACE_NET

#include <memory>
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
        virtual bool Start(absl::string_view ip, uint16_t port) = 0;
        // heart beat
        virtual void SendHeartRequest(absl::string_view net_handle, HeartBeatResquest& request) = 0;
        virtual void SendHeartResponse(absl::string_view net_handle, HeartBeatResponse& response) = 0;
        // vote
        virtual void SendVoteRequest(absl::string_view net_handle, VoteRequest& request) = 0;
        virtual void SendVoteResponse(absl::string_view net_handle, VoteResponse& response) = 0;

        // set new connect call back
        virtual void SetNewConnectCallBack(absl::FunctionRef<void(absl::string_view net_handle)> func) = 0;
        // set disconnect call back
        virtual void SetDisConnectCallBack(absl::FunctionRef<void(absl::string_view net_handle)> func) = 0;
        // set heart request call back
        virtual void SetHeartRequestRecvCallBack(absl::FunctionRef<void(absl::string_view, HeartBeatResquest&)> func) = 0;
        // set heart response call back
        virtual void SetHeartResponseRecvCallBack(absl::FunctionRef<void(absl::string_view, HeartBeatResponse&)> func) = 0;
        // set vote request call back
        virtual void SetVoteRequestRecvCallBack(absl::FunctionRef<void(absl::string_view, VoteRequest&)> func) = 0;
        // set vote response call back
        virtual void SetVoteResponseRecvCallBack(absl::FunctionRef<void(absl::string_view, VoteResponse&)> func) = 0;
        
    };
}

#endif