#ifndef RAFT_INTERFACE_COMMIYLOG
#define RAFT_INTERFACE_COMMIYLOG

#include <vector>
#include <string>
#include "absl/functional/function_ref.h"
#include "absl/strings/string_view.h"

namespace raft {
    class CCommitLog {
    public:
        CCommitLog() {}
        virtual ~CCommitLog() {}

        // submit log that after merge.
        virtual bool PushLog(absl::string_view log) = 0;
        // submit log with index.
        virtual void PushLog(uint64_t index, absl::string_view log) = 0;
        // submit log with index and term
        virtual void PushLog(uint64_t index, uint32_t term, absl::string_view log) = 0;
        // get log vector after index
        virtual bool GetLog(uint64_t index, std::vector<std::string>& log_vec) = 0;
        // sync get log after index.
        virtual bool GetLog(uint64_t index) = 0;
        //set get log call back
        virtual void SetGetLogCallBack(absl::FunctionRef<void(std::vector<std::string>&)> func) = 0;
        //get the newest index
        virtual uint64_t GetNewestIndex() = 0;
    };
}
#endif