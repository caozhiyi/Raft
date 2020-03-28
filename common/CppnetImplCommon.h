#ifndef RAFT_CPMMON_CPPNETIMPLCOMMON
#define RAFT_CPMMON_CPPNETIMPLCOMMON

#include <map>

namespace raft {
    enum CppBagType {
        heart_beat_request  = 1,
        heart_beat_response = 2,
        vote_request        = 3,
        vote_response       = 4,
        entries_requst      = 5,
        entries_response    = 6,
        node_info_request   = 7,
        node_info_response  = 8
    };

    static const uint32_t __header_len = sizeof(uint64_t);
    struct CppBag {
        union Header {
            struct Field {
                uint32_t  _len;
                uint32_t  _type;
            } _field;
            uint64_t      _data;
        } _header;
        std::string       _body;
    };
}

#endif