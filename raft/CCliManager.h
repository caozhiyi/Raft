#ifndef HEADER_CCLI_MANAGER
#define HEADER_CCLI_MANAGER

#include <mutex>
#include "common.h"

namespace google {
    namespace protobuf {
        class Closure;
    }
}

namespace raft {
    class ClientResponse;
    class CNode;

    class CCliManager
    {
    public:
        CCliManager(CNode* node);
    	~CCliManager();
        
        // handle a msg from client
        void HandleClient(const std::string& ip_port, const std::string& msg, ::raft::ClientResponse* response,
            ::google::protobuf::Closure* done);
        // send a response to a client by a time 
        bool SendRet(Time& time_pos, int err_code, const std::string& des);
        // send all client the cur node is not a leader
        void SendNotLeader();
    
    private:
        CNode*          _node;
        std::mutex      _client_mutex;
        std::map<Time, std::pair<void*, void*>>  _client_msg;   // msg version and client ip 
    };
}
#endif