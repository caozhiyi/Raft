#ifndef HEADER_CCLIENT
#define HEADER_CCLIENT

#include <mutex>
#include <functional>

#include "common.h"
#include "config.h"

typedef std::function<void(int, const std::string&)> ClientCallBack;

namespace brpc {
    class Controller;
}
namespace raft {
    class ClientService_Stub;
    class ClientResponse;

    class CClient
    {
    public:
        CClient();
        ~CClient();

        bool Init(const std::string& config_file);

        void SetCallBackFunc(const ClientCallBack& call_back);
    
        bool SendMsg(const std::string& msg);

        static void RpcDone(void* param, brpc::Controller* cntl);

    private:
        std::string _leader_info;
        CConfig     _config;
        raft::ClientService_Stub* _channel;
        ClientCallBack _client_call_back;
    };
}

#endif