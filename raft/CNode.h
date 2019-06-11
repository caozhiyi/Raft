#ifndef HEADER_CNODE
#define HEADER_CNODE

#include <mutex>
#include <atomic>
#include <functional>

#include "CHeart.h"
#include "CLogReplication.h"
#include "common.h"
#include "config.h"
#include "CCliManager.h"
#include "CSevManager.h"
#include "CMsgRouter.h"
#include "Log.h"

namespace raft {
    typedef std::function<void(std::string)> MsgCallBack;

    class CNode
    {
    public:
        CNode(const std::string& config_path);
        ~CNode();

        bool Init(int log_level = LOG_ERROR_LEVEL);
        // set msg call back
        void SetMsgCallBack(MsgCallBack& func);
        // load info from config file again
        bool LoadConfig();
        // the local node is leader now?
        bool IsLeader();
        // get cur leader ip info
        std::string GetLeaderInfo();
        // send heart to all node. only leader call this function
        void SendAllHeart();
        // handle heart request from other node
        void HandleHeart(const std::string& ip_port, std::vector<std::string>& msg_vec, long long version, bool done_msg, long long& new_version);
        // handle vote request from other node
        void HandleVote(const std::string& ip_port, long long version, bool& vote);
        // get a msg from client return the time of msg
        Time HandleClientMsg(const std::string& ip_port, const std::string& msg);
        // leader node sync msg to other node
        void Sync(std::vector<std::pair<std::string, Time>>& sync_vec);
        // clean msg cache
        void CleanMsg();

    private:
        // timer call back function
        void _HeartCallBack();
        void _TimeOutCallBack();

    private:
        NodeRole	  _role;
        std::string   _leader_ip_port;
        std::string   _config_path;

        CMsgRouter    _msg_router;
        CConfig		  _config;
        CHeart		  _heart;
        CBinLog       _bin_log;
        CCliManager   _cli_manager;
        CSevManager   _sev_manager;

        MsgCallBack   _msg_call_back;
    
        std::atomic<bool> _done_msg;	    // notice follower done msg to file

        std::mutex _msg_mutex;
        std::vector<std::string>	_cur_msg;
        std::vector<std::string>	_will_done_msg;
    };
}
#endif