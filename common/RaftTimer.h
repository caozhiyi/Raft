
#ifndef RAFT_COMMON_TIMER
#define RAFT_COMMON_TIMER

#include <map>
#include <atomic>
#include <condition_variable>

#include "ITimer.h"
#include "Runnable.h"

namespace raft {

    class CTimerImpl : public CTimer, public base::CRunnable {
    public:
        CTimerImpl();
        ~CTimerImpl();
        // start timer thread
        void Start();
        // reset all timer
        void ResetTimer();
        // stop timer thread
        void Stop();
        // add vote timer out call back
        void SetVoteCallBack(TimerSoltRef func);
        // add heart timer out call back
        void SetHeartCallBack(TimerSoltRef func);
        // start vote timer
        bool StartVoteTimer(uint32_t time);
        // heart timer
        bool StartHeartTimer(uint32_t time);
        void StopHeartTimer();
    private:
        // thread function
        void Run();
        // add timer event
        bool AddTimer(uint32_t time, TimerType type);

    private:
        // all expiration in list
        std::map<uint64_t, TimerType>    _timer_map;
        TimerSolt                        _vote_call_back;
        TimerSolt                        _heart_call_back;
        uint32_t                         _heart_time;
        // cur wait time
        uint64_t                         _wait_time;
        // thread safe
        std::mutex					     _mutex;
        std::condition_variable  	     _notify;
    };
}

#endif