#include <exception>
#include "Entries.h"
using namespace raft;

EntriesRef::EntriesRef(Entries& entries) {
    _data._field._index = entries._index;
    _data._field._term  = entries._term;
    _data._field._entries = (char*)entries._entries.c_str();
    _data._field._total_len = entries._entries.length() + __field_len;
}

EntriesRef::EntriesRef(char* data, uint32_t len) {
    if (len <= __field_len) {
        throw std::exception(std::logic_error("data len is too small."));
    }
    _data._data = data;
}

EntriesRef::EntriesRef(uint32_t term, uint64_t index, char* entries, uint32_t len) {
    _data._field._term = term;
    _data._field._index = index;
    _data._field._total_len = len + sizeof(uint32_t) + sizeof(uint64_t);
    _data._field._entries = entries;
}

EntriesRef::~EntriesRef() {

}

Entries EntriesRef::GetEntries() {
    Entries entries;
    entries._index = _data._field._index;
    entries._term  = _data._field._term;
    entries._entries = std::string(_data._field._entries, _data._field._total_len - sizeof(uint32_t) - sizeof(uint64_t));
    return std::move(entries);
}

uint32_t EntriesRef::GetTerm() {
    return _data._field._term;
}

uint64_t EntriesRef::GetIndex() {
    return _data._field._index;
}

uint32_t EntriesRef::GetTotalLen() {
    return _data._field._total_len;
}

void EntriesRef::GetEntriesContent(char* entries, uint32_t& len) {
    entries = _data._field._entries;
    len = _data._field._total_len - sizeof(uint32_t) - sizeof(uint64_t);
}

void EntriesRef::GetData(char* data, uint32_t& len) {
    data = _data._data;
    len = _data._field._total_len;
}