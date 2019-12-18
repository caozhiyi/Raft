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
        virtual bool Start(const std::string& ip, uint16_t port) = 0;
        virtual void Join() = 0;
        // heart beat
        virtual void SendHeartRequest(const std::string& net_handle, HeartBeatResquest& request) = 0;
        virtual void SendHeartResponse(const std::string& net_handle, HeartBeatResponse& response) = 0;
        // vote
        virtual void SendVoteRequest(const std::string& net_handle, VoteRequest& request) = 0;
        virtual void SendVoteResponse(const std::string& net_handle, VoteResponse& response) = 0;

        // client about
        virtual void SendToClient(const std::string& net_handle, ClientResponse& response) = 0;
        // client call back
        virtual void SetClientRecvCallBack(absl::FunctionRef<void(const std::string&, ClientRequest& request)> func) = 0;
        virtual void SetClientConnectCallBack(absl::FunctionRef<void(const std::string&)> func) = 0;
        virtual void SetClientDisConnectCallBack(absl::FunctionRef<void(const std::string&)> func) = 0;

        // set new connect call back
        virtual void SetNewConnectCallBack(absl::FunctionRef<void(absl::string_view)> func) = 0;
        // set disconnect call back
        virtual void SetDisConnectCallBack(absl::FunctionRef<void(absl::string_view)> func) = 0;
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