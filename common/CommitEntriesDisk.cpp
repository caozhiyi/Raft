#include <cstring> // for memcpy
#include "CommitEntriesDisk.h"
#include "Log.h"

static const int __once_step = 4096 * 2;    // read size every time

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
    char *buf = new char[entries._entries.length()];
    memcpy(buf, entries.data(), entries.length());
    EntriesRef entries_ref(buf, entries.length());
    uint32_t len = 0;
    char* data = nullptr;
    entries_ref.GetData(data, len);
    mmrev(data, data + len);
    delete[] buf;
    return WriteEntries(data, len);
}

bool CCommitEntriesDisk::PushEntries(uint64_t index, uint32_t term, absl::string_view entries) {
    char *buf = new char[entries.length()];
    memcpy(buf, entries.data(), entries.length());
    EntriesRef entries_ref(index, term, buf, entries.length());
    uint32_t len = 0;
    char* data = nullptr;
    entries_ref.GetData(data, len);
    mmrev(data, data + len);
    delete[] buf;
    return WriteEntries(data, len);
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

void CCommitEntriesDisk::SetEntriesCallBack(absl::FunctionRef<void(std::vector<Entries>&)> func) {
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
	size_t file_size = _in_file_stream.tellg();
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
    uint32_t find_rand = 0;
    bool ret = true;
    
	while (true) {
        find_rand++;
        // create buf
        cur_buf = new char[__once_step];
        buf_vec.push_back(cur_buf);

        // if have prev
        if (prev_buf && prev_buf_left) {
            memcpy(cur_buf, prev_buf, prev_buf_left);
        }

        _in_file_stream.read(cur_buf + prev_buf_left, __once_step - prev_buf_left);
		int len = _in_file_stream.gcount();
		if (len == 0) {
			break;
		}
        mmrev(cur_buf + prev_buf_left, cur_buf + prev_buf_left + len);

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

bool CCommitEntriesDisk::WriteEntries(char* data, int32_t len) {
    _out_file_stream << std::string(data, len);
    _out_file_stream.flush();
}