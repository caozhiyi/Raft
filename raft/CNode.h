#ifndef HEADER_CNODE
#define HEADER_CNODE

#include <mutex>

#include "baidu/rpc/channel.h"
#include "CHeart.h"
#include "CLogReplication.h"
#include "common.h"
#include "config.h"

class CNode
{
public:
	CNode(const std::string& config_path);
	~CNode();

    bool Init();

	bool LoadConfig();

	// only leader send heart
	void SendAllHeart();
	// time out. to Candidate
	void SendAllVote();

	bool IsLeader();

	void HandleHeart(const std::string& ip_port, std::vector<std::string>& msg_vec, long long version, bool done_msg, long long& new_version);

	void HandleVote(const std::string& ip_port, long long version, bool& vote);
    
    Time HandleClientMsg(const std::string& ip_port, const std::string& msg);
    // broadcast node info to all node
    void BroadCastNodeInfo();
    // get current node's node info
    void GetNodeInfo(const std::string& ip_port, std::vector<std::string>& node_info);
    // add new node to connection map(_channel_map)
    void AddNewNode(const std::string& ip_port, const std::vector<std::string>& node_info);
    // leader node sync msg to other node
    void Sync();
    // clean msg cache
    void CleanMsg();

private:
	// timer
	void _HeartCallBack();
	void _TimeOutCallBack();

private:
	NodeRole	_role;
	
	std::string _config_path;
	CConfig		_config;
	CHeart		_heart;
	CBinLog     _bin_log;

	std::string _leader_ip_port;
    std::string _local_ip_port;

    std::atomic_bool _done_msg;	    // notice follower done msg to file

	std::mutex _msg_mutex;
	std::vector<std::string>	_cur_msg;
    std::vector<std::string>	_will_done_msg;              // only leader use
    std::vector<std::pair<Time, std::string>> _client_msg;   // msg version and client ip

    std::vector<std::pair<std::string, Time>>       _sync_vec;

	std::mutex _stub_mutex;
	std::map<std::string, raft::RaftService_Stub*>	_channel_map;
};

#endif