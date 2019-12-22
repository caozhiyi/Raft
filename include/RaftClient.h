#ifndef RAFT_INCLUDE_RAFTCLIENT
#define RAFT_INCLUDE_RAFTCLIENT

#include <cstdint>
#include <functional>

namespace raft {
    enum ERR_CODE {
        client_success = 0,
        client_error   = 1
    };
    // init raft library.
    // thread_num : the number of running threads.
    void Init(const std::string& config_file = "./conf/raft_client.conf");
    void Dealloc();

    // thread join.
    void Join();

    // send entries
    void SendEntries(const std::string& entries);
    // set commit entries call back.
    void SetCommitEntriesCallBack(std::function<void(ERR_CODE)> func);
}

#endif
