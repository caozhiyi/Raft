#ifndef RAFT_RAFT_CLIENTMANAGER
#define RAFT_RAFT_CLIENTMANAGER

#include "IClientManager.h"

namespace raft {

    class CNet;
    class CRole;
    class CClient;
    class CClientManagerImpl : public CClientManager {
    public:
        CClientManagerImpl(std::shared_ptr<CNet>& net);
        virtual ~CClientManagerImpl();
        
        void SetRole(std::shared_ptr<CRole>& role);
        // send response to all client
        void SendToAll(ClientResponse& response);

        // client call back
        void RecvClientRequest(const std::string& net_handle, ClientRequest& request);
        void ClientConnect(const std::string& net_handle);
        void ClientDisConnect(const std::string& net_handle);

    private:
        // all client
        std::map<std::string, std::shared_ptr<CClient>> _client_map;
        // currnet role ptr
        std::shared_ptr<CRole> _current_role;
        // net 
        std::shared_ptr<CNet>  _net;
    };
}

#endif