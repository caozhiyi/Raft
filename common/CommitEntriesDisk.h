#ifndef RAFT_COMMON_COMMIYLOGNORMAL
#define RAFT_COMMON_COMMIYLOGNORMAL

#include <fstream>
#include <functional>
#include <vector>

#include "absl/functional/function_ref.h"
#include "Entries.h"
#include "ICommitEntries.h"

namespace raft {
    class CCommitEntriesDisk : public CCommitEntries {
    public:
        CCommitEntriesDisk(const std::string& file_name);
        virtual ~CCommitEntriesDisk();

        // submit entries that after merge.
        bool PushEntries(const Entries& entries);
        // submit entries with index and term
        bool PushEntries(uint64_t index, uint32_t term, const std::string& entries);
        // get entries vector after index
        bool GetEntries(uint64_t index, std::vector<Entries>& entries_vec);
        // sync get entries after index.
        bool GetEntries(uint64_t index);
        //set get entries call back
        void SetEntriesCallBack(absl::FunctionRef<void(std::vector<Entries>&)> func);
        //get the newest index
        uint64_t GetNewestIndex();
    private:
        // only get one entries
        bool GetEntries(Entries& entries_vec);
        bool ReadEntries(uint64_t index, std::vector<Entries>& entries_vec, bool only_one = false);
        void WriteEntries(const std::string& entries);

    private:
        std::string            _file_name;
        std::fstream           _in_file_stream;
        std::fstream           _out_file_stream;
        uint64_t               _newest_index;
        std::function<void(std::vector<Entries>&)> _entries_call_back;
    };
}
#endif