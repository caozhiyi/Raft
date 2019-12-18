#ifndef RAFT_RAFT_CLIENTIMPL
#define RAFT_RAFT_CLIENTIMPL

#include "IClient.h"

namespace raft {

    class CNet;
    class CClientImpl : public CClient {
    public:
        CClientImpl(std::shared_ptr<CNet>& net, const std::string& handle);
        virtual ~CClientImpl();
        
        // net handle
        virtual void SetNetHandle(const std::string& handle);
        virtual std::string GetNetHandle();

        // send response to client
        virtual void SendToClient(ClientResponse& response);
    private:
        std::shared_ptr<CNet>           _net;
        std::string                     _net_handle;
    };
}

#endif