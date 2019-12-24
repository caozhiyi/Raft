#ifndef RAFT_INCLUDE_RAFT
#define RAFT_INCLUDE_RAFT

#include <cstdint>
#include <functional>

namespace raft {
    // init raft library.
    // thread_num : the number of running threads.
    void Init(const std::string& config_file = "./conf/raft.conf");
    void Dealloc();

    // thread join.
    void Join();

    // push entries to cluser
    void PushEntries(const std::string& entries);
    // set commit entries call back.
    void SetCommitEntriesCallBack(std::function<void(std::string)> func);
}

#endif
