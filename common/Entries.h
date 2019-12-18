#ifndef RAFT_COMMON_ENTRIES
#define RAFT_COMMON_ENTRIES

#include <string>
namespace raft {
    
    static const int __field_len = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(char*);

    struct Entries {
        uint32_t _term;
        uint64_t _index;
        std::string _entries;
    };
    // entries ref.
    class EntriesRef {
    public:
        EntriesRef(const Entries& entries);
        EntriesRef(char* data, uint32_t len);
        EntriesRef(uint32_t term, uint64_t index, char* entries, uint32_t len);
        ~EntriesRef();

        Entries GetEntries();
        uint32_t GetTerm();
        uint64_t GetIndex();
        uint32_t GetTotalLen();
        std::string GetEntriesContent();
        std::string GetData();

    private:
        union Data {
            struct Field {
                uint32_t _total_len;
                uint32_t _term;
                uint64_t _index;
                char*    _entries;
            } _field;

            char         _data[__field_len];
        } _data;
    };
}
#endif