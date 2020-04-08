#ifndef RAFT_CPMMON_NETCOMMON
#define RAFT_CPMMON_NETCOMMON

#include <map>

namespace raft {
    enum MessageType {
        heart_beat_request  = 1,
        heart_beat_response = 2,
        vote_request        = 3,
        vote_response       = 4,
        entries_requst      = 5,
        entries_response    = 6,
        node_info_request   = 7,
        node_info_response  = 8
    };
}

#endif