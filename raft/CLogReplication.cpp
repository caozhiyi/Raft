#include <exception>
#include <string.h>         //for memset
#include "CLogReplication.h"
using namespace raft;

#define FLAG_STR "%R%N"
#define FLAG_LEN 4
#define ONCE_LEN 1024

CBinLog::CBinLog(std::string fine_name) : _file_name(fine_name) {
    _out_file_stream.open(_file_name, std::ios::app | std::ios::out);
    _in_file_stream.open(_file_name, std::ios::in);
	if (!_in_file_stream.good() || !_out_file_stream.good()) {
		throw std::exception(std::logic_error("open log file failed"));
	}

    std::vector<std::string> vec = GetTargetLogStr(1);
    if (vec.empty()) {
        _newest_time = 0;

    } else {
        BinLog bin_log = StrToBinLog(vec[0]);
        _newest_time = bin_log.first;
    }
}

CBinLog::~CBinLog() {
    _in_file_stream.close();
    _out_file_stream.close();
}

void CBinLog::Run() {
    while(!_stop) {
        BinLog log = _Pop();
        if (log.first == 0) {
            break;
        }
		std::string log_str = BinLogToStr(log);

		std::unique_lock<std::mutex> lock(_mutex);
        _out_file_stream << FLAG_STR;
        _out_file_stream << log_str;
        _out_file_stream.flush();
    }
}

void CBinLog::Stop() {
	_stop = true;
    BinLog bin_log;
    bin_log.first = 0;
    Push(std::move(bin_log));
    // wait the sub thread stop
    Join();
}

bool CBinLog::PushLog(const std::string& log) {
    int pos = log.find(':');
    if (pos == std::string::npos) {
        return false;
    }
    BinLog bin_log = StrToBinLog(log);
    _newest_time = bin_log.first;
    Push(std::move(bin_log));
    return true;
}

void CBinLog::PushLog(Time time, std::string log) {
    BinLog bin_log;
	bin_log.first  = time;
	bin_log.second = log;
	_newest_time = time;
    Push(std::move(bin_log));
}

bool CBinLog::GetLog(Time time, std::vector<BinLog>& log_vec) {
	std::vector<std::string> vec = GetTargetLogStr(100);
	if (vec.empty()) {
		return false;
	}
	for (size_t i = 0; i < vec.size(); i++) {
		BinLog bin_log = StrToBinLog(vec[i]);
		if (bin_log.first <= time) {
			break;
		}
		log_vec.push_back(std::move(bin_log));
	}
	return true;
}

Time CBinLog::GetNewestTime() {
	return _newest_time;
}

Time CBinLog::GetUTC() {
	return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
}

std::vector<std::string> CBinLog::GetTargetLogStr(int count) {
    std::vector<std::string> res_vec;
    std::unique_lock<std::mutex> lock(_mutex);
    _in_file_stream.clear();
    _in_file_stream.seekg(0, std::ios::end);
	size_t file_size = _in_file_stream.tellg();
    if (file_size == 0) {
        return res_vec;
    }
	if (file_size <= ONCE_LEN){
        _in_file_stream.seekg(0, std::ios::beg);
	} else {
        _in_file_stream.seekg(-ONCE_LEN, std::ios::end);
	}
	bool ret = false;
	int pri_pos = 0;
	int find_rand = 1;
	char buf[ONCE_LEN * 2 + 1] = { 0 };
	char pri_buf[ONCE_LEN + 1] = { 0 };
	while (true) {
		find_rand ++;
		memset(buf, 0, ONCE_LEN * 2 + 1);
        _in_file_stream.read(buf, ONCE_LEN);
		int len = _in_file_stream.gcount();
		if (len == 0) {
			break;
		}
		if (pri_pos > 0) {
			strncpy(buf + len, pri_buf, pri_pos);
			memset(pri_buf, 0, ONCE_LEN + 1);
			pri_pos = len + pri_pos;
		} else {
			pri_pos = len;
		}
		
		char cur[FLAG_LEN+1] = { 0 };
		for (int i = len - FLAG_LEN; i >= 0; i--) {
			strncpy(cur, buf+i, FLAG_LEN);
			if (strcmp(cur, FLAG_STR) == 0) {
				res_vec.push_back(std::string(buf + i + FLAG_LEN, pri_pos - i - FLAG_LEN));
				pri_pos = i;
				count--;
				if (count == 0) {
					ret = true;
					break;
				}
			}
		}
		if (ret) {
			break;
		}
		if (pri_pos > 0) {
			strncpy(pri_buf, buf, pri_pos);
		}
		if (_in_file_stream.eof()) {
            _in_file_stream.clear();
		}
		int pos = -find_rand*ONCE_LEN;
        _in_file_stream.seekg(pos, std::ios::end);
	}
    _in_file_stream.seekg(0, std::ios::end);
	return res_vec;
}

BinLog CBinLog::StrToBinLog(const std::string& log) {
	int pos = log.find(':');
	BinLog res;
    res.first = 0;
	if (pos == std::string::npos) {
		return res;
	}
	std::string time_srt = log.substr(0, pos);
	std::string content = log.substr(pos + 1, log.length());

	res.first  = atoll(time_srt.c_str());
	res.second = std::move(content);
	return std::move(res);
}

std::string CBinLog::BinLogToStr(const BinLog& log) {
	std::string res;
	res.append(std::to_string(log.first));
	res.append(":");
	res.append(log.second);
	return std::move(res);
}