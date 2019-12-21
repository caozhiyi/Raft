#ifndef RAFT_INTERFACE_COMMIYLOG
#define RAFT_INTERFACE_COMMIYLOG

#include <vector>
#include <string>

#include "absl/functional/function_ref.h"
#include "Entries.h"

namespace raft {
    class CCommitEntries {
    public:
        CCommitEntries() {}
        virtual ~CCommitEntries() {}

        // submit log that after merge.
        virtual bool PushEntries(const Entries& entries) = 0;
        // submit log with index and term
        virtual bool PushEntries(uint64_t index, uint32_t term, const std::string& entries) = 0;
        // get log vector after index
        virtual bool GetEntries(uint64_t index, std::vector<Entries>& entries_vec) = 0;
        // sync get log after index.
        virtual bool GetEntries(uint64_t index) = 0;
        //set get log call back
        virtual void SetEntriesCallBack(absl::FunctionRef<void(std::vector<Entries>&)> func) = 0;
        //get the newest index
        virtual uint64_t GetNewestIndex() = 0;
    };
}
#endif