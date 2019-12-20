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
    class RaftService_Stub;
    class ClientResponse;

    class CClient
    {
    public:
        CClient();
        ~CClient();

        bool Init(const std::string& config_file = "");

        void SetCallBackFunc(const ClientCallBack& call_back);
    
        bool SendMsg(const std::string& msg);

        static void RpcDone(void* param, brpc::Controller* cntl);

        // test connect is valid
        bool SayHello(RaftService_Stub* stub);

    private:
        std::string _leader_info;
        CConfig     _config;
        std::string _config_path;
        raft::RaftService_Stub* _ser_stub;
        ClientCallBack _client_call_back;
    };
}

#endif