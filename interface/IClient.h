#ifndef RAFT_INTERFACE_CLIENT
#define RAFT_INTERFACE_CLIENT

#include "message.pb.h"

namespace raft {

    class CClient {
    public:
        CClient() {}
        virtual ~CClient() {}
        
        // net handle
        virtual void SetNetHandle(const std::string& handle) = 0;
        virtual std::string GetNetHandle() = 0;

        // send response to client
        virtual void SendToClient(ClientResponse& response) = 0;
    };
}

#endif