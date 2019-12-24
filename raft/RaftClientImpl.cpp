#include "Log.h"
#include "Runnable.h"
#include "CppnetImpl.h"
#include "ConfigImpl.h"
#include "RaftClientImpl.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"

using namespace raft;

CRaftClientImpl::CRaftClientImpl() {
    _net.reset(new CCppNet());
    _net->SetClientResponseCallBack(std::bind(&CRaftClientImpl::ClientResponseCallBack, this, std::placeholders::_1, std::placeholders::_2));
    _net->SetClientConnectCallBack(std::bind(&CRaftClientImpl::ClientConnect, this, std::placeholders::_1));
    _net->SetClientDisConnectCallBack(std::bind(&CRaftClientImpl::ClientDisConnect, this, std::placeholders::_1));
    _net->SetNodeInfoResponseCallBack(std::bind(&CRaftClientImpl::NodeInfoResponseCallBack, this, std::placeholders::_1, std::placeholders::_2));
}

CRaftClientImpl::~CRaftClientImpl() {

}

void CRaftClientImpl::Init(const std::string& config_file) {
    _config.reset(new CConfigImpl(config_file));

    // print log?
    bool print_log = _config->PrintLog();
    if (print_log) {
        std::string log_file = _config->GetLogFile();
        uint16_t log_level = _config->GetLogLevel();
        base::CLog::Instance().SetLogLevel((base::LogLevel)log_level);
        base::CLog::Instance().SetLogName(log_file);
        base::CLog::Instance().Start();
    }

    // start net io
    std::string ip = _config->GetIp();
    uint16_t port = _config->GetPort();
    uint16_t thread_num = _config->GetThreadNum();
    _net->Init(thread_num);

    // connect to other node
    std::string node_address = _config->GetNodeInfo();
    std::vector<std::string> addr_vec = absl::StrSplit(node_address, ";");
    for (size_t i = 0; i < addr_vec.size(); i++) {
        if (_net_handle_set.count(addr_vec[i]) == 0) {
            continue;
        }
        _net_handle_set.insert(addr_vec[i]);
    }

    auto iter = _net_handle_set.begin();
    std::vector<std::string> handle_vec = absl::StrSplit(*iter, ":");
    if (handle_vec.size() == 2) {
        uint32_t port = 0;
        if (!absl::SimpleAtoi<uint32_t>(handle_vec[1], &port)) {
            base::LOG_ERROR("parser net handle failed.");
        }
        _net->ConnectTo(handle_vec[0], (uint16_t)port);
    }
}

void CRaftClientImpl::Dealloc() {
    _net->Dealloc();
}

void CRaftClientImpl::Join() {
    _net->Join();
}

void CRaftClientImpl::SendEntries(const std::string& entries) {
    ClientRequest reqeust;
    reqeust.set_entries(entries);
    _resend_entries = entries;
    _net->SendClientRequest(_cur_net_handle, reqeust);
}

void CRaftClientImpl::SetCommitEntriesCallBack(std::function<void(ERR_CODE)> func) {
    _response_call_back = func;
}

void CRaftClientImpl::ClientConnect(const std::string& net_handle) {
    if (!_cur_net_handle.empty()) {
        _net->DisConnect(net_handle);
    }
    _cur_net_handle = net_handle;
    if (!_resend_entries.empty()) {
        SendEntries(_resend_entries);
        _resend_entries.clear();
    }
    NodeInfoRequest request;
    _net->SendNodeInfoRequest(_cur_net_handle, request);
}

void CRaftClientImpl::ClientDisConnect(const std::string& net_handle) {
    _cur_net_handle.clear();
    _net_handle_set.erase(net_handle);
    if (_net_handle_set.size()) {
        auto iter = _net_handle_set.begin();
        std::vector<std::string> handle_vec = absl::StrSplit(*iter, ":");
        if (handle_vec.size() == 2) {
            uint32_t port = 0;
            if (!absl::SimpleAtoi<uint32_t>(handle_vec[1], &port)) {
                base::LOG_ERROR("parser net handle failed.");
            }
            _net->ConnectTo(handle_vec[0], (uint16_t)port);
        }
    }
}

void CRaftClientImpl::ClientResponseCallBack(const std::string& net_handle, ClientResponse& response) {
    auto ret_code = response.ret_code();
    if (ret_code == success) {
        _response_call_back(client_success);
        _resend_entries.clear();
    
    } else if (ret_code == not_leader) {
        _net->DisConnect(_cur_net_handle);
        std::string leader = response.leader_net_handle();
        std::vector<std::string> handle_vec = absl::StrSplit(leader, ":");
        if (handle_vec.size() == 2) {
            uint32_t port = 0;
            if (!absl::SimpleAtoi<uint32_t>(handle_vec[1], &port)) {
                base::LOG_ERROR("parser net handle failed.");
            }
            _net->ConnectTo(handle_vec[0], (uint16_t)port);
        }

    } else if (ret_code == other_error) {
        _response_call_back(client_error);
        _resend_entries.clear();
        
    } else if (ret_code == send_again) {
        base::CRunnable::Sleep(100);
        SendEntries(_resend_entries);
    }
}

void CRaftClientImpl::NodeInfoResponseCallBack(const std::string& net_handle, NodeInfoResponse& response) {
    auto size = response.net_handle_size();
    for (int i = 0; i < size; i++) {
        std::string net_handle = response.net_handle(i);
        if (_net_handle_set.count(net_handle) != 0) {
            continue;
        }
        _net_handle_set.insert(std::move(net_handle));
    }
}