#include <cstring>      // for memcpy
#include <algorithm>    // for reverse

#include "Log.h"
#include "CommitEntriesDisk.h"

static const int32_t __once_step = 4096 * 2;    // read size every time

using namespace raft;

static void mmrev(char* start, char* end) {
    if (start == nullptr || end == nullptr) {
        return;
    }
    while(1) {
        std::swap(*start, *end);
        start++;
        if (start == end) {
            break;
        }
        end--;
        if (start == end) {
            break;
        }
    }
}

CCommitEntriesDisk::CCommitEntriesDisk(const std::string& file_name) {
    _file_name = file_name;
    _out_file_stream.open(_file_name, std::ios::binary | std::ios::app | std::ios::out);
    _in_file_stream.open(_file_name, std::ios::binary | std::ios::in);
	if (!_in_file_stream.good() || !_out_file_stream.good()) {
		throw std::exception(std::logic_error("open entries file failed"));
	}

    // get newest index
    Entries new_entries;
    if (GetEntries(new_entries)) {
        _newest_index = new_entries._index;
    } else {
        _newest_index = 0;
    }
}

CCommitEntriesDisk::~CCommitEntriesDisk() {
    if (_in_file_stream) {
        _in_file_stream.close();
    }
    if (_out_file_stream) {
        _out_file_stream.close();
    }
}

bool CCommitEntriesDisk::PushEntries(const Entries& entries) {
    if (_newest_index >= entries._index && entries._index != 0) {
        return false;
    }
    _newest_index = entries._index;
    EntriesRef entries_ref(entries);
    
    std::string data = entries_ref.GetData();
    std::reverse(data.begin(), data.end());
    WriteEntries(data);
    return true;
}

bool CCommitEntriesDisk::PushEntries(uint64_t index, uint32_t term, const std::string& entries) {
    if (_newest_index >= index && index != 0) {
        return false;
    }
    _newest_index = index;

    EntriesRef entries_ref(term, index, (char*)entries.data(), entries.length());

    std::string data = entries_ref.GetData();
    std::reverse(data.begin(), data.end());
    WriteEntries(data);
    return true;
}

bool CCommitEntriesDisk::GetEntries(uint64_t index, std::vector<Entries>& entries_vec) {
    return ReadEntries(index, entries_vec);
}

bool CCommitEntriesDisk::GetEntries(uint64_t index) {
    std::vector<Entries> entries_vec;
    bool ret = ReadEntries(0, entries_vec, true);
    if (ret && !entries_vec.empty()) {
        if (_entries_call_back) {
            _entries_call_back(entries_vec);
        }
        return true;
    }
    return false;
}

void CCommitEntriesDisk::SetEntriesCallBack(const std::function<void(std::vector<Entries>&)>& func) {
    _entries_call_back = func;
}

uint64_t CCommitEntriesDisk::GetNewestIndex() {
    return _newest_index;
}

bool CCommitEntriesDisk::GetEntries(Entries& entries) {
    std::vector<Entries> entries_vec;
    bool ret = ReadEntries(0, entries_vec, true);
    if (ret && !entries_vec.empty()) {
        entries = entries_vec[0];
        return true;
    }
    return false;
}

bool CCommitEntriesDisk::ReadEntries(uint64_t index, std::vector<Entries>& entries_vec, bool only_one) {
    _in_file_stream.clear();
    _in_file_stream.seekg(0, std::ios::end);
	auto file_size = _in_file_stream.tellg();
    if (file_size == 0) {
        return false;
    }
	if (file_size <= __once_step){
        _in_file_stream.seekg(0, std::ios::beg);
	} else {
        _in_file_stream.seekg(-__once_step, std::ios::end);
	}

    std::vector<char*> buf_vec;
    char* cur_buf = nullptr;
    char* prev_buf = nullptr;
    uint32_t prev_buf_left = 0;
    int32_t find_rand = 0;
    bool ret = true;
    
    bool loop = true;
	while (loop) {
        find_rand++;
        // create buf
        cur_buf = new char[__once_step];
        buf_vec.push_back(cur_buf);

        // if have prev
        if (prev_buf && prev_buf_left) {
            memcpy(cur_buf, prev_buf, prev_buf_left);
        }

        _in_file_stream.read(cur_buf + prev_buf_left, __once_step - prev_buf_left);
		auto len = _in_file_stream.gcount();
		if (len == 0) {
			break;
		}
        mmrev(cur_buf + prev_buf_left, cur_buf + prev_buf_left + len -1);

        uint32_t left_len = prev_buf_left + len;
        uint32_t offset = 0;
        // parser current buffer.
        while (true) {
            if (left_len > __field_len) {
			    EntriesRef entries_ref(cur_buf + offset, left_len);
                if (entries_ref.GetTotalLen() > left_len) {
                    prev_buf = cur_buf;
                    prev_buf_left = left_len;
                    break;
                } else {
                    // get a complete entries.
                    entries_vec.push_back(std::move(entries_ref.GetEntries()));
                    if (entries_ref.GetIndex() <= index || only_one) {
                        loop = false;
                        break;
                    }
                    offset += entries_ref.GetTotalLen();
                    left_len -= entries_ref.GetTotalLen();
                }
		    } else {
                prev_buf = cur_buf;
                prev_buf_left = left_len;
                break;
            }
        }

		if (_in_file_stream.eof()) {
            _in_file_stream.clear();
		}
        // move read point
		int pos = -find_rand * __once_step;
        _in_file_stream.seekg(pos, std::ios::end);
	}
    // reset read point to end of file.
    _in_file_stream.seekg(0, std::ios::end);
    for (size_t i = 0; i < buf_vec.size(); i++) {
        delete[] buf_vec[i];
    }
    
    return ret;
}

void CCommitEntriesDisk::WriteEntries(const std::string& entries) {
    _out_file_stream << entries;
    _out_file_stream.flush();
}