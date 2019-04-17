#include "Log.h"
#include "CNode.h"
#include "CZkNodeInfo.h"
#include "CParser.h"

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
	
    int listen_port = _config.GetIntValue("listen_port");
    _local_port = _config.GetIntValue("local_port");

	std::string local_ip_port_str = _local_ip + ":" + std::to_string(_local_port);
	
	// connect all node
	{
		std::string ip_port;
		std::unique_lock<std::mutex> lock(_socket_mutex);
		for (auto iter = node_info_list.begin(); iter != node_info_list.end(); ++iter) {
			auto soket = _net.Connection(iter->_port, iter->_ip);
			ip_port = iter->_ip + ":" + std::to_string(iter->_port);
			if (soket) {
				_socket_map["ip_port"] = soket;

			} else {
				LOG_ERROR("connect node failed. ip:%s, port:%d", iter->_ip.c_str(), iter->_port);
				return false;
			}
		}
	}

	// start timer
    _heart.SetHeartCallBack(std::bind(&CNode::_HeartCallBack, this));
    _heart.SetTimeOutCallBack(std::bind(&CNode::_TimeOutCallBack, this));
	int step = _config.GetIntValue("timer_step");
	int heart_time = _config.GetIntValue("heart_time");
	int time_out = _config.GetIntValue("time_out");
	LOG_INFO("Init heart timer. timer_step:%d, heart_time:%d, time_out:%d", step, heart_time, time_out);
	_heart.Init(step, heart_time, time_out);

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
    if (_done_msg) {
        request.set_done_msg(_done_msg);
        _done_msg = false;
    }

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
    Time version = _bin_log.GetNewestTime();
    request.set_version(version);

    int back_count = 1;
    {
        std::unique_lock<std::mutex> lock(_stub_mutex);
        for (auto iter = _channel_map.begin(); iter != _channel_map.end(); ++iter) {
            iter->second->RpcNodeHeart(&cntl, &request, &response, NULL);
            if (response->version() != new_version) {
                _synv_vec.push_back(std::make_pair(iter->first, response->version()));

            } else {
                back_count++;
            }
        }
    }
	
    if (back_count  >= _socket_map.size() / 2) {
        _done_msg = true;
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
        iter->second->RpcNodeVote(&cntl, &request, &response, NULL);
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

    if (done_msg) {
        for (size_t i = 0; i < _will_done_msg.size(); i++) {
            _bin_log.PushLog(_will_done_msg[i]);
        }
        _will_done_msg.clear();
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

void CNode::HandleVote(const std::string& ip_port, bool& vote) {
    if (_role == Leader) {
        vote = true;
        _heart.ResetTimer();

    } else if (_role == Follower) {
        vote = true;
        _heart.ResetTimer();

    } else if (_role == Candidate) {
        // do nothing
    }
}

void CNode::Sync() {
    raft_rpc::HeartResponse response;
    baidu::rpc::Controller cntl;
    cntl.set_timeout_ms(500);
    for (auto iter = _synv_vec.begin(); iter != _synv_vec.end(); ++iter) {
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
            stub->RpcNodeHeart(&cntl, &request, &response, NULL);
        }
    }
    _synv_vec.clear();
}

void CNode::HandleClient(const BinLog& log) {
	if (_role == Leader) {
        std::string log_str = _bin_log.BinLogToStr(log);
		std::unique_lock<std::mutex> lock(_msg_mutex);
		_cur_msg.push_back(std::move(log_str));
	}
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