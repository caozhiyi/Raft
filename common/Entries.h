#ifndef RAFT_COMMON_BASELOG
#define RAFT_COMMON_BASELOG

#include <string>
namespace raft {
    
    static const int __field_len = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint64_t);

    struct Entries {
        uint32_t _term;
        uint64_t _index;
        std::string _entries;
    };
    // entries ref.
    class EntriesRef {
    public:
        EntriesRef(Entries& entries);
        EntriesRef(char* data, uint32_t len);
        EntriesRef(uint32_t term, uint64_t index, char* entries, uint32_t len);
        ~EntriesRef();

        Entries GetEntries();
        uint32_t GetTerm();
        uint64_t GetIndex();
        uint32_t GetTotalLen();
        void GetEntriesContent(char* entries, uint32_t& len);
        void GetData(char* data, uint32_t& len);

    private:
        union Data {
            struct Field {
                uint32_t _total_len;
                uint32_t _term;
                uint64_t _index;
                char*    _entries;
            } _field;

            char*        _data;
        } _data;
    };
}
#endif