#ifndef HEADER_CNODE
#define HEADER_CNODE

#include <mutex>

#include "baidu/rpc/channel.h"
#include "raft_rpc.pb.h"
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
	void HandleVote(const std::string& ip_port, bool& vote);
    void Sync();
private:
	// timer
	void _HeartCallBack();
	void _TimeOutCallBack();

private:
	NodeRole	_role;
	
	std::string _config_path;
	CConfig		_config;

	CHeart		_heart;
	std::atomic_int _msg_re_count;	// reheart' num
	CBinLog     _bin_log;
	std::string _leader_ip_port;

    std::atomic_bool _done_msg;	    // notice follower done msg to file

	std::string _local_ip;
	int			_local_port;

	std::mutex _msg_mutex;
	std::vector<std::string>	_cur_msg;
    std::vector<std::string>	_will_done_msg;

    std::vector<std::pair<std::string, Time>> _synv_vec;

	std::mutex _stub_mutex;
	std::map<std::string, std::shared_ptr<raft::RaftService_Stub*>>	_channel_map;
};

#endif