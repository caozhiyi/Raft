#include <algorithm>
#include "CNode.h"
#include "UnixOs.h"

using namespace raft;

CNode::CNode(const std::string& config_path) :
    _done_msg(false),
	_role(Follower),
    _cli_manager(this),
	_config_path(config_path) {

}

CNode::~CNode() {
    _bin_log.Stop();
    _heart.Stop();
}

bool CNode::Init(int log_level) {
    // start log thread
    CLog::Instance().SetLogName("server.log");
    CLog::Instance().SetLogLevel((LogLevel)log_level);
    CLog::Instance().Start();

	// begin log thread
	_bin_log.Start();
	if (!LoadConfig()) {
        LOG_ERROR("load config file failed.");
        return false;
	}

    _msg_router.Init(this, &_sev_manager, &_cli_manager);
      
    // start timer
    _heart.SetHeartCallBack(std::bind(&CNode::_HeartCallBack, this));
    _heart.SetTimeOutCallBack(std::bind(&CNode::_TimeOutCallBack, this));
    int step = _config.GetIntValue("timer_step");
    int heart_time = _config.GetIntValue("heart_time");
    int time_out = _config.GetIntValue("time_out");
    _heart.Init(step, heart_time, time_out);

    LOG_INFO("Init heart timer. timer_step:%d, heart_time:%d, time_out:%d", step, heart_time, time_out);

    // init sev manager
    if (!_sev_manager.Init(_config, &_msg_router)) {
        LOG_ERROR("init sev manager failed.");
        return false;
    }
	return true;
}

void CNode::SetMsgCallBack(MsgCallBack& func) {
    _msg_call_back = func;
}

bool CNode::LoadConfig() {
	return _config.LoadFile(_config_path);
}

void CNode::SendAllHeart() {
    // send to other node info
    bool done_msg = false;
    std::vector<std::string> msg_vec;
    Time new_version = 0;
    Time log_version = 0;
    std::vector<std::pair<std::string, Time>> sync_vec;

    // leader write msg to file
    if (_done_msg) {
        done_msg = true;
         for (size_t i = 0; i < _will_done_msg.size(); i++) {
            _bin_log.PushLog(_will_done_msg[i]);
            BinLog bin_log = _bin_log.StrToBinLog(_will_done_msg[i]);
            // send client response
            _cli_manager.SendRet(bin_log.first, ERR_Success, "success");
         }
         _will_done_msg.clear();
        _done_msg = false;
    }

    // get current newest version
	{
		std::unique_lock<std::mutex> lock(_msg_mutex);
        for (auto iter = _cur_msg.begin(); iter != _cur_msg.end(); ++iter) {
            msg_vec.push_back(*iter);
            _will_done_msg.push_back(*iter);
        }
        if (!_cur_msg.empty()) {
            BinLog bin_log = _bin_log.StrToBinLog(_cur_msg[_cur_msg.size() - 1]);
            new_version = bin_log.first;
            _cur_msg.clear();
        }
	}
    if (new_version == 0) {
        new_version = _bin_log.GetNewestTime();
    }
    // get the pri newest version
    log_version = _bin_log.GetNewestTime();

    if (_sev_manager.SendAllHeart(log_version, new_version, msg_vec, done_msg, sync_vec)) {
        _done_msg = true;

    } else {
        // send client response
        for (auto iter = _cur_msg.begin(); iter != _cur_msg.end(); ++iter) {
            BinLog bin_log = _bin_log.StrToBinLog(_cur_msg[_cur_msg.size() - 1]);
            _cli_manager.SendRet(bin_log.first, ERR_LessNodeRecv, "less half node recv the msg");
        }
        _cur_msg.clear();
    }

    Sync(sync_vec);
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
            if (_msg_call_back) {
                _msg_call_back(_cur_msg[i]);
            }
        }
        _cur_msg.clear();
    }

	// cur node lost message. sync from loeader
    if (_cur_msg.size() > 0) {
        BinLog bin_log = _bin_log.StrToBinLog(_cur_msg[_cur_msg.size() - 1]);
        if (version > bin_log.first) {
            new_version = _bin_log.GetNewestTime();
            LOG_INFO("send a sync msg to leader : %s", ip_port.c_str());
            return;
        }
        
    } else if (version > _bin_log.GetNewestTime() && _bin_log.GetNewestTime() != 0) {
        new_version = _bin_log.GetNewestTime();
		LOG_INFO("send a sync msg to leader : %s", ip_port.c_str());
		return;
	}

	// get messsage
	if (msg_vec.size() > 0) {
		std::unique_lock<std::mutex> lock(_msg_mutex);
		for (size_t i = 0; i < msg_vec.size(); i++) {
			LOG_INFO("recv a msg from leader : %s", msg_vec[i].c_str());
            _cur_msg.push_back(std::move(msg_vec[i]));
		}
	}

	// set response
    if (_cur_msg.size() > 0) {
        BinLog bin_log = _bin_log.StrToBinLog(_cur_msg[_cur_msg.size() - 1]);
        new_version = bin_log.first;

    } else {
        new_version = _bin_log.GetNewestTime();
    }
    LOG_DEBUG("send a reheart msg to leader : %s", ip_port.c_str());
}
                      
void CNode::HandleVote(const std::string& ip_port, long long version, bool& vote) {
    if (_role == Leader) {
        // current leader is powerful
        if (_bin_log.GetNewestTime() > version && version != 0) {
            SendAllHeart();
        // vote
        } else {
            vote = true;
            _heart.ResetTimer();
        }

    } else if (_role == Follower) {
        vote = true;
        _heart.ResetTimer();
        _leader_ip_port = ip_port;

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

void CNode::Sync(std::vector<std::pair<std::string, Time>>& sync_vec) {
    std::vector<std::string> msg_vec;
    for (auto iter = sync_vec.begin(); iter != sync_vec.end(); ++iter) {
       
        std::vector<BinLog> log_vec;
        _bin_log.GetLog(iter->second, log_vec);
        if (log_vec.size() > 0) {
            reverse(log_vec.begin(), log_vec.end());
            for (size_t i = 0; i < log_vec.size(); i++) {
                std::string msg = _bin_log.BinLogToStr(log_vec[i]);
                msg_vec.push_back(msg);
            }
        }
        for (size_t i = 0; i < _will_done_msg.size(); i++) {
            msg_vec.push_back(_will_done_msg[i]);
        }
        
        if (msg_vec.size() == 0) {
            continue;
        }
        _sev_manager.SendSyncHeart(iter->first, iter->second, std::move(msg_vec));
    }
}

void CNode::CleanMsg() {
    _cli_manager.SendNotLeader();
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
		LOG_DEBUG("heart call back.");
	}
    //LoadConfig();
}

void CNode::_TimeOutCallBack() {
	if (_role != Leader) {
        _role = Candidate;
        if (_sev_manager.SendAllVote(_bin_log.GetNewestTime())) {
            _role = Leader;
            SendAllHeart();
        }
	} 
    LOG_DEBUG("time out call back to be a candidate.");
}