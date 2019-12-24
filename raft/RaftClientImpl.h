#ifndef RAFT_RAFT_RAFTCLIENTIMPL
#define RAFT_RAFT_RAFTCLIENTIMPL

#include "Single.h"
#include "RaftClient.h"
#include "message.pb.h"

namespace raft {

    class CNet;
    class CConfig;
    class CRaftClientImpl : public base::CSingle<CRaftClientImpl> {
    public:
        CRaftClientImpl();
        ~CRaftClientImpl();
        // init raft library.
        // thread_num : the number of running threads.
        void Init(const std::string& config_file);
        void Dealloc();

        // thread join.
        void Join();

        // send entries
        void SendEntries(const std::string& entries);
        // set commit entries call back.
        void SetCommitEntriesCallBack(std::function<void(ERR_CODE)> func);
    
        // connect about
        void ClientConnect(const std::string& net_handle);
        void ClientDisConnect(const std::string& net_handle);
        // client response call back
        void ClientResponseCallBack(const std::string& net_handle, ClientResponse& response);
        // get all node info
        void NodeInfoResponseCallBack(const std::string& net_handle, NodeInfoResponse& response);
    private:
        std::shared_ptr<CNet>           _net;
        std::shared_ptr<CConfig>        _config;
        std::set<std::string>           _net_handle_set;
        std::string                     _cur_net_handle;
        std::string                     _resend_entries;
        std::function<void(ERR_CODE)>   _response_call_back;
    };
}

#endif