#include "Log.h"
#include "CNode.h"
#include "Tool.h"

#include <iostream>

CNode::CNode(const std::string& config_path) :
    _done_msg(false),
	_role(Follower),
	_config_path(config_path) {

}

CNode::~CNode() {
    _bin_log.Stop();
}

bool CNode::Init() {
	// begin log thread
	_bin_log.Start();
	if (!LoadConfig()) {
        LOG_ERROR("load config file failed.");
        return false;
	}
	
    // init net. begin listen
    _local_ip_port = _config.GetIntValue("local");
    
    baidu::rpc::Server server;
    server.set_version("raft_server_1.0");
    raft::CNodeRpc node(this);
    if (server.AddService(&node, baidu::rpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        LOG_ERROR("fail to add service.");
        return false;
    }

    baidu::rpc::ServerOptions options;
    if (server.Start(_local_ip_port.c_str(), &options) != 0) {
        LOG_ERROR("Fail to start storage_server. ip : %s", _local_ip_port.c_str());
        return false;
    }

    // is first node?
    bool is_first_node = _config.GetIntValue("is_first_node");
    
    
    if (!is_first_node) {
        // add a new node to cluster
        std::vector<std::string> node_info_list;
        std::string node_list = _config.GetIntValue("node_list");
        node_info_list = SplitStr(node_list, ";");

        baidu::rpc::Channel *channel = new  baidu::rpc::Channel;
        baidu::rpc::ChannelOptions options;
        std::string connect_node_ip;
        for (auto iter = node_info_list.begin(); iter != node_info_list.end(); ++iter) {
            if (channel.Init(iter->c_str(), &options) != 0) {
                LOG_ERROR("connect a new node failed. ip : %s", iter->c_str());
            
            } else {
                connect_node_ip = std::move(*iter);
                break;
            }
        }

        if (connect_node_ip.empty()) {
            LOG_ERROR("connect a new node failed. but current node is't the firstly.");
            return false;
        }
        
        raft::RaftService_Stub *stub = new raft::RaftService_Stub(channel);
        _channel_map[connect_node_ip] = stub;

        // get all node info
        raft::NodeInfoRequest request;
        raft::NodeInfoResponse response;
        baidu::rpc::Controller cntl;
        cntl.set_timeout_ms(500);
        stub->rpc_node_info(&cntl, &request, &response, NULL);

        std::vector<std::string> info_vec;
        for (int i = 0; i < response.ip_port_size(); i++) {
            info_vec.push_back(response.ip_port(i));
        }
        // connect all node
        AddNewNode(connect_node_ip, info_vec);

        BroadCastNodeInfo();
    }
    
    // start timer
    _heart.SetHeartCallBack(std::bind(&CNode::_HeartCallBack, this));
    _heart.SetTimeOutCallBack(std::bind(&CNode::_TimeOutCallBack, this));
    int step = _config.GetIntValue("timer_step");
    int heart_time = _config.GetIntValue("heart_time");
    int time_out = _config.GetIntValue("time_out");
    LOG_INFO("Init heart timer. timer_step:%d, heart_time:%d, time_out:%d", step, heart_time, time_out);
    _heart.Init(step, heart_time, time_out);

    server.RunUntilAskedToQuit();
	return true;
}

bool CNode::LoadConfig() {
	return _config.LoadFile(_config_path);
}

void CNode::SendAllHeart() {
    raft_rpc::HeartRequest request;
    raft_rpc::HeartResponse response;
    baidu::rpc::Controller cntl;
    cntl.set_timeout_ms(500);

    // leader write msg to file
    if (_done_msg) {
         for (size_t i = 0; i < _will_done_msg.size(); i++) {
            _bin_log.PushLog(_will_done_msg[i]);
            BinLog bin_log = _bin_log.StrToBinLog(_will_done_msg[i]);
            // send client response
            _listener.SendRet(bin_log.first, ERR_Success, "success");
         }
         _will_done_msg.clear();
        request.set_done_msg(_done_msg);

        _done_msg = false;
    }

    // get current newest version
    Time new_version = 0;
	{
		std::unique_lock<std::mutex> lock(_msg_mutex);
        for (auto iter = _cur_msg.begin(); iter != _cur_msg.end(); ++iter) {
            request.add_msg(*iter);
            _will_done_msg.push_back(*iter);
        }
        BinLog bin_log = _bin_log.StrToBinLog(_cur_msg[_cur_msg.size() - 1]);
        new_version = bin_log.first;
        _cur_msg.clear();
	}

    // get the pri newest version
    Time version = _bin_log.GetNewestTime();
    request.set_version(version);

    int back_count = 1;
    {
        std::unique_lock<std::mutex> lock(_stub_mutex);
        for (auto iter = _channel_map.begin(); iter != _channel_map.end(); ++iter) {
            iter->second->rpc_heart(&cntl, &request, &response, NULL);
            // neew sync
            if (response->version() != new_version) {
                _sync_vec.push_back(std::make_pair(iter->first, response->version()));

            } else {
                back_count++;
            }
        }
    }
	
    if (back_count  >= _socket_map.size() / 2) {
        _done_msg = true;
    
    } else {
        // send client response
        for (auto iter = _cur_msg.begin(); iter != _cur_msg.end(); ++iter) {
            BinLog bin_log = _bin_log.StrToBinLog(_cur_msg[_cur_msg.size() - 1]);
            _listener.SendRet(bin_log.first, ERR_LessNodeRecv, "less half node recv the msg");
        }
        _cur_msg.clear();
    }

    Sync();
}

void CNode::SendAllVote() {
    ::raft_rpc::VoteResuest request;
    ::raft_rpc::VoteToResponse response;
    baidu::rpc::Controller cntl;
    cntl.set_timeout_ms(500);

    int vote_count = 0;	// vote's num

	std::unique_lock<std::mutex> lock(_socket_mutex);
    for(auto iter = _channel_map.begin(); iter != _channel_map.end(); ++iter) {
        iter->second->rpc_vote(&cntl, &request, &response, NULL);
        if (response->vote()) {
            vote_count++;
        } 
    }
    
    // to be a leader
    if (vote_count >= _channel_map.size() / 2) {
        _role = Leader;
        SendAllHeart();
    }
}

bool CNode::IsLeader() {
	return _role == Leader;
}

std::string CNode::GetLeaderInfo() {
	return _leader_ip_port;
}

void CNode::HandleHeart(const std::string& ip_port, std::vector<std::string>& msg_vec, long long version, bool done_msg, long long& new_version) {
	if (_role != Follower) {
		_role = Follower;
		_leader_ip_port = ip_port;
		LOG_INFO("Leader change to %s", ip_port.c_str());
	}

	_heart.ResetTimer();

    // follower write msg to file
    if (done_msg) {
        std::unique_lock<std::mutex> lock(_msg_mutex);
        for (size_t i = 0; i < _cur_msg.size(); i++) {
            _bin_log.PushLog(_cur_msg[i]);
        }
        _cur_msg.clear();
    }

	// cur node lost message. sync from loeader
	if (version != _bin_log.GetNewestTime() && _bin_log.GetNewestTime() != 0) {
		Time version = _bin_log.GetNewestTime();
		
        new_version = version;
		LOG_INFO("send a sync msg to leader : %s", ip_port.c_str());
		return;
	}

	// get messsage
	if (msg_vec.size() > 0) {
		std::unique_lock<std::mutex> lock(_msg_mutex);
		for (size_t i = 0; i < msg_vec.size(); i++) {
			LOG_INFO("recv a msg from client : %s", msg_vec[i].c_str());
            _cur_msg.push_back(std::move(msg_vec[i]));
		}
	}

	// set response
	BinLog bin_log = _bin_log.StrToBinLog(_cur_msg[_cur_msg.size() - 1]);
	Time version = bin_log.first;
    LOG_INFO("send a reheart msg to leader : %s", ip_port.c_str());
}

void CNode::HandleVote(const std::string& ip_port, long long version, bool& vote) {
    if (_role == Leader) {
        // current leader is powerful
        if (_bin_log.GetNewestTime() >= version) {
            SendAllHeart();
        // vote
        } else {
            vote = true;
            _heart.ResetTimer();
        }

    } else if (_role == Follower) {
        vote = true;
        _heart.ResetTimer();

    } else if (_role == Candidate) {
        // do nothing
    }
}

Time CNode::HandleClientMsg(const std::string& ip_port, const std::string& msg) {
    if (_role == Leader) {
        BinLog log;
        log.first = _bin_log.GetUTC();
        log.second = msg;
        std::string log_str = _bin_log.BinLogToStr(log);
        std::unique_lock<std::mutex> lock(_msg_mutex);
        _cur_msg.push_back(std::move(log_str));
        return log.first;
    }
}

void CNode::BroadCastNodeInfo() {
    raft::NodeInfoRequest request;
    raft::NodeInfoResponse response;
    baidu::rpc::Controller cntl;
    cntl.set_timeout_ms(500);

    std::unique_lock<std::mutex> lock(_stub_mutex);
    for (auto iter = _channel_map.begin(); iter != _channel_map.end(); ++iter) {
        request->add_ip_port(iter->first);
    }
    for (auto iter = _channel_map.begin(); iter != _channel_map.end(); ++iter) {
        iter->second->rpc_node_info(&cntl, &request, &response, NULL);
    } 
}

void CNode::GetNodeInfo(const std::string& ip_port, std::vector<std::string>& node_info) {
    // connect the new node
    baidu::rpc::Channel *channel = new  baidu::rpc::Channel;
    baidu::rpc::ChannelOptions options;
    if (channel.Init(ip_port.c_str(), &options) != 0) {
        LOG_ERROR("connect a new node failed. ip : %s", ip_port.c_str());
        return;
    }
    raft::RaftService_Stub *stub = new raft::RaftService_Stub(channel);
    std::unique_lock<std::mutex> lock(_stub_mutex);
    _channel_map[ip_port] = stub;

    // get all old node
    for (auto iter = _channel_map.begin(); iter != _channel_map.end(); ++iter) {
        node_info.add_ip_port(iter->first);
    }
    return;
}

void CNode::AddNewNode(const std::string& ip_port, const std::vector<std::string>& node_info) {
    std::unique_lock<std::mutex> lock(_stub_mutex);
    // add all node info
    for (size_t i = 0; i < node_info.size(); i++) {
        if (_channel_map.find(node_info[i] != _channel_map.end())) {
            continue;
        }

        baidu::rpc::Channel *channel = new  baidu::rpc::Channel;
        baidu::rpc::ChannelOptions options;
        if (channel.Init(node_info[i].c_str(), &options) != 0) {
            LOG_ERROR("connect a new node failed. ip : %s", node_info[i].c_str());
            continue;
        }

        raft::RaftService_Stub *stub = new raft::RaftService_Stub(channel);
        _channel_map[node_info[i]] = stub;
    }
}

void CNode::Sync() {
    raft_rpc::HeartResponse response;
    baidu::rpc::Controller cntl;
    cntl.set_timeout_ms(500);
    for (auto iter = _sync_vec.begin(); iter != _sync_vec.end(); ++iter) {
        raft_rpc::HeartRequest request;
        request.set_done_msg(false);
        request.set_version(iter->second);
        std::vector<BinLog> log_vec;
        _bin_log.GetLog(iter->second, log_vec);
        for (size_t i = 0; i < log_vec.size(); i++) {
            request.add_msg(std::move(_bin_log.BinLogToStr(log_vec[i])));
        }
        for (size_t i = 0; i < _will_done_msg.size(); i++) {
            request.add_msg(_will_done_msg[i]);
        }
        auto stub = _channel_map.find(iter->first);
        if (stub) {
            stub->rpc_heart(&cntl, &request, &response, NULL);
        }
    }
    _sync_vec.clear();
}

void CNode::CleanMsg() {
    CleanMsg();
    _done_msg = false;
    std::unique_lock<std::mutex> lock(_msg_mutex);
    _cur_msg.clear();
    _will_done_msg.clear();
}

// timer
void CNode::_HeartCallBack() {
	if (_role == Leader) {
		_heart.ResetTimer();
		SendAllHeart();
		LoadConfig();
		LOG_INFO("heart call back.");
	}
}

void CNode::_TimeOutCallBack() {
	if (_role != Leader) {
        _role = Candidate;
        SendAllVote();
		LOG_INFO("to be a candidate.");
	} 
    LOG_INFO("time out call back.");
}