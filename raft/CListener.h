#ifndef HEADER_CNODE
#define HEADER_CNODE

#include <mutex>

#include "common.h"

namespace raft_rpc {
    class ClientResponse;
}
namespace google {
    namespace protobuf {
        class Closure;
    }
}

class CNode;
class CListener
{
public:
    CListener();
	~CListener();

    bool Init(const std::string& ip_port, CNode* node);

    void HandleClient(const std::string& ip_port, const std::string& msg, ::raft_rpc::ClientResponse* response,
        ::google::protobuf::Closure* done);

    bool SendRet(Time& time_pos, int err_code, const std::string& des);

    void CleanMsg();

private:
    CNode*          _node;
    std::string     _ip_port;
    std::mutex      _client_mutex;
    std::map<Time, std::pair<void*, void*>>  _client_msg;   // msg version and client ip 
};

#endif