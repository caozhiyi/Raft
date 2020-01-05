#include <vector>
#include "RaftTimer.h"
#include "absl/time/time.h"
#include "absl/time/clock.h"

using namespace raft;
static const uint32_t __timer_empty_wait = 100000000;

CTimerImpl::CTimerImpl() : _heart_time(0), _wait_time(0), _reset(false){

}

CTimerImpl::~CTimerImpl() {
    Stop();
    _notify.notify_one();
    Join();
}
    
void CTimerImpl::Start() {
    CRunnable::Start();
}
    
void CTimerImpl::ResetTimer() {
    _reset = true;
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _timer_map.clear();
    }
    _notify.notify_one();
}
    
void CTimerImpl::Stop() {
    CRunnable::Stop();
    _notify.notify_one();
}

void CTimerImpl::SetVoteCallBack(TimerSoltRef func) {
    _vote_call_back = func;
}

void CTimerImpl::SetHeartCallBack(TimerSoltRef func) {
    _heart_call_back = func;
}

bool CTimerImpl::StartVoteTimer(uint32_t time) {
    return AddTimer(time, vote_timer);
}

bool CTimerImpl::StartHeartTimer(uint32_t time) {
    _heart_time = time;
    return AddTimer(time, heart_timer);
}

void CTimerImpl::StopHeartTimer() {
    _heart_time = 0;
}

void CTimerImpl::Run() {
    std::vector<TimerType> timer_vec;
    std::map<uint64_t, TimerType>::iterator iter;
    bool timer_out = false;
    while (!_stop) {
        _reset = false;
        {
            iter = _timer_map.end();
            timer_out = false;
            std::unique_lock<std::mutex> lock(_mutex);
            if (_timer_map.empty()) {
                _wait_time = __timer_empty_wait;

            } else {
                iter = _timer_map.begin();
                timer_vec.push_back(iter->second);
                uint64_t now_time = absl::ToUnixMillis(absl::Now());
                if (iter->first > now_time) {
                    _wait_time = iter->first - now_time;

                } else {
                    _wait_time = 0;
                    timer_out = true;
                }
            }
            if (_wait_time > 0) {
                timer_out = _notify.wait_for(lock, std::chrono::milliseconds(_wait_time)) == std::cv_status::timeout;
            }
        }

        if (timer_vec.size() > 0 && !_reset) {
            for (size_t i = 0; i < timer_vec.size(); i++) {
                if (timer_vec[i] == vote_timer) {
                    if (iter != _timer_map.end()) {
                        _timer_map.erase(iter);
                    }
                    _vote_call_back();

                } else if (timer_vec[i] == heart_timer) {
                    if (iter != _timer_map.end()) {
                        _timer_map.erase(iter);
                    }
                    if (_heart_time > 0) {
                        _heart_call_back();
                        AddTimer(_heart_time, heart_timer);
                    }
                }
            }
        }
        timer_vec.clear();
    }
}

bool CTimerImpl::AddTimer(uint32_t time, TimerType type) {
    uint64_t now_time = absl::ToUnixMillis(absl::Now());
    uint64_t expiration_time = time + now_time;

    std::unique_lock<std::mutex> lock(_mutex);
    auto iter = _timer_map.find(expiration_time);
    // add to timer map
    if (iter == _timer_map.end()) {
        _timer_map[expiration_time] = type;

    // add same time
    } else {
        return false;
    }
    if (time < _wait_time) {
        _notify.notify_one();
    }
    return true;
}