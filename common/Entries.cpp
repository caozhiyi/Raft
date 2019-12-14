#include <exception>
#include "Entries.h"
using namespace raft;

static const uint32_t __char_point_size = sizeof(char*);
static const uint32_t __flex_total_size = __field_len - __char_point_size;

EntriesRef::EntriesRef(const Entries& entries) {
    _data._field._index = entries._index;
    _data._field._term  = entries._term;
    _data._field._entries = (char*)entries._entries.c_str();
    _data._field._total_len = entries._entries.length() + __flex_total_size;
}

EntriesRef::EntriesRef(char* data, uint32_t len) {
    if (len < __field_len) {
        throw std::exception(std::logic_error("data len is too small."));
    }
    memcpy(_data._data, data, __flex_total_size);
    _data._field._entries = data + __flex_total_size;
}

EntriesRef::EntriesRef(uint32_t term, uint64_t index, char* entries, uint32_t len) {
    _data._field._term = term;
    _data._field._index = index;
    _data._field._total_len = len + __flex_total_size;
    _data._field._entries = entries;
}

EntriesRef::~EntriesRef() {

}

Entries EntriesRef::GetEntries() {
    Entries entries;
    entries._index = _data._field._index;
    entries._term  = _data._field._term;
    entries._entries = std::string(_data._field._entries, _data._field._total_len - __flex_total_size);
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

std::string EntriesRef::GetEntriesContent() {
    return std::move(std::string(_data._field._entries, _data._field._total_len - __flex_total_size));
}

std::string EntriesRef::GetData() {
    std::string ret(_data._data, __flex_total_size);
    ret.append(_data._field._entries, _data._field._total_len - __flex_total_size);
    return std::move(ret);
}