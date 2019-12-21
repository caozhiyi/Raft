#ifndef RAFT_INTERFACE_CLIENTMANAGER
#define RAFT_INTERFACE_CLIENTMANAGER

#include "message.pb.h"

namespace raft {

    class CRole;
    class CClientManager {
    public:
        CClientManager() {}
        virtual ~CClientManager() {}

        virtual void SetRole(std::shared_ptr<CRole>& role) = 0;
        virtual void SendToAll(ClientResponse& response) = 0;      
        // client call back
        virtual void RecvClientRequest(const std::string& net_handle, ClientRequest& request) = 0;
        virtual void ClientConnect(const std::string& net_handle) = 0;
        virtual void ClientDisConnect(const std::string& net_handle) = 0;
    };
}

#endif