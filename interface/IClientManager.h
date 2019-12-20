#ifndef RAFT_INTERFACE_CLIENTMANAGER
#define RAFT_INTERFACE_CLIENTMANAGER

#include "message.pb.h"
#include "absl/strings/string_view.h"

namespace raft {

    class CRole;
    class CClientManager {
    public:
        CClientManager() {}
        virtual ~CClientManager() {}

        virtual void SetRole(std::shared_ptr<CRole>& role) = 0;
        virtual void SendToAll(ClientResponse& response) = 0;      
        // client call back
        virtual void RecvClientRequest(absl::string_view net_handle, ClientRequest& request) = 0;
        virtual void ClientConnect(absl::string_view net_handle) = 0;
        virtual void ClientDisConnect(absl::string_view net_handle) = 0;  
    };
}

#endif