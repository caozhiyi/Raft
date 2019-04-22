#ifndef HEADER_CNODE
#define HEADER_CNODE

#include <mutex>

#include "common.h"

class CNode;
class CListener
{
public:
    CListener();
	~CListener();

    bool Init(const std::string& ip_port);

    void HandleClient(const std::string& ip_port, const std::string& msg, ::raft_rpc::ClientResponse* response,
        ::google::protobuf::Closure* done);

    bool SendRet(const std::string& ip_port, int err_code, const std::string& des);

private:
    CNode* _node;
    std::string _ip_port;
    
    std::map<Time, std::pair<void*, void*>>  _client_msg;   // msg version and client ip 
};

#endif