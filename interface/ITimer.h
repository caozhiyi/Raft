#ifndef RAFT_INTERFACE_TIMER
#define RAFT_INTERFACE_TIMER

#include <functional>
#include "absl/functional/function_ref.h"

namespace raft {

    enum TimerType{
            vote_timer  = 0x01,
            heart_timer = 0x02
    };

    typedef std::function<void(void)> TimerSolt;
    typedef absl::FunctionRef<void(void)> TimerSoltRef;
    
    class CTimer {
    public:
        CTimer() {}
        virtual ~CTimer() {}

        // start timer thread
        virtual void Start() = 0;
        // reset all timer
        virtual void ResetTimer() = 0;
        // stop timer thread
        virtual void Stop() = 0;
        // add vote timer out call back
        virtual void SetVoteCallBack(TimerSoltRef func) = 0;
        // add heart timer out call back
        virtual void SetHeartCallBack(TimerSoltRef func) = 0;
        // start vote timer
        virtual bool StartVoteTimer(uint32_t time) = 0;
        // heart timer
        virtual bool StartHeartTimer(uint32_t time) = 0;
        virtual void StopHeartTimer() = 0;
    };
}

#endif