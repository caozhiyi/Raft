#ifndef HEADER_CSRV_MANAGER
#define HEADER_CSRV_MANAGER

#include <mutex>
#include <brpc/channel.h>
#include "common.h"
#include "config.h"

namespace raft {

    class RaftService_Stub;
    class CMsgRouter;
    class CSevManager
    {
    public:
        CSevManager();
    	~CSevManager();
        
        bool Init(CConfig& config, CMsgRouter* router);
        // send sync heart info to only one node
        bool SendSyncHeart(std::string ip_info, Time log_version, std::vector<std::string> msg_vec);
        // only leader send heart
        // ret true: half of node recv msg
        bool SendAllHeart(Time log_version, Time new_version, std::vector<std::string> msg_vec,
                          bool done_msg, std::vector<std::pair<std::string, Time>>& sync_vec);
        // time out. to Candidate
        bool SendAllVote(Time log_versoin);
        // broadcast node info to all node
        void BroadCastNodeInfo();
        // get current node's node info
        void GetNodeInfo(const std::string& ip_port, std::vector<std::string>& node_info);
        // add new node to connection map(_channel_map)
        void AddNewNode(const std::string& ip_port, const std::vector<std::string>& node_info);
        // test connect is valid
        bool SayHello(RaftService_Stub* stub);
    
    private:
        std::string     _local_ip_port;
        std::mutex      _stub_mutex;
        std::map<std::string, raft::RaftService_Stub*>	_channel_map;
    };
}
#endif