#ifndef HEADER_CLISTENER
#define HEADER_CLISTENER

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
    class CListener
    {
    public:
        CListener(CNode* node);
    	~CListener();
 
        void HandleClient(const std::string& ip_port, const std::string& msg, ::raft::ClientResponse* response,
            ::google::protobuf::Closure* done);
    
        bool SendRet(Time& time_pos, int err_code, const std::string& des);
    
        void CleanMsg();
    
    private:
        CNode*          _node;
        std::string     _ip_port;
        std::mutex      _client_mutex;
        std::map<Time, std::pair<void*, void*>>  _client_msg;   // msg version and client ip 
    };
}
#endif