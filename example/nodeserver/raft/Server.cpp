#include <iostream>

#include "Log.h"
#include "Raft.h"
#include "server.h"
#include "absl/strings/str_split.h"
#include "absl/strings/str_format.h"

void CServer::Init(const std::string& config_file) {
    raft::Init(config_file);
    _http_server.Init("127.0.0.1", 8900);


    raft::SetCommitEntriesCallBack(std::bind(&CServer::RecvEntries, this, std::placeholders::_1));
    _http_server.SetRequestCallBack(std::bind(&CServer::DoRequest, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void CServer::Join() {
    _http_server.Join();
    raft::Join();
}

std::string CServer::DoRequest(RequestType type, const std::string& key, const std::string& value) {
    if (key.empty()) {
        return "must have key.";
    }

    if (type == request_query) {
        std::cout << "query key:" << key << std::endl;
        std::unique_lock<std::mutex> lock(_mutex);
        auto iter = _data_map.find(key);
        if (iter == _data_map.end()) {
            return "can't find key.";

        }
        else {
            return iter->second;
        }

    }
    else {
        if (type == request_add || type == request_modify) {
            if (value.empty()) {
                return "add and modify must have value.";
            }
        }
        std::string entries = absl::StrFormat("%d|%s|%s", (int)type, key.c_str(), value.c_str());
        raft::PushEntries(entries);
        return "success";
    }
}

void CServer::RecvEntries(std::string entries) {
    std::vector<std::string> vec;
    vec = absl::StrSplit(entries, "|");
    if (vec.size() != 3) {
        base::LOG_ERROR("recv a error entries.");
        return;
    }

    RequestType type = (RequestType)atoi(vec[0].c_str());
    if (type == request_add) {
        std::cout << "add key:" << vec[1] << std::endl;
        std::unique_lock<std::mutex> lock(_mutex);
        _data_map[vec[1]] = vec[2];

    }
    else if (type == request_remove) {
        std::cout << "remove key:" << vec[1] << std::endl;
        std::unique_lock<std::mutex> lock(_mutex);
        _data_map.erase(vec[1]);

    }
    else if (type == request_modify) {
        std::cout << "modify key:" << vec[1] << std::endl;
        std::unique_lock<std::mutex> lock(_mutex);
        _data_map[vec[1]] = vec[2];

    }
    else {
        base::LOG_ERROR("get a unknow type.");
    }
}