#include "Runnable.h"
#include "CppnetImpl.h"
#include "ConfigImpl.h"
#include "ClientRaftImpl.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"

using namespace raft;

CClientRaftImpl::CClientRaftImpl() {
    _net.reset(new CCppNet());
    _net->SetClientResponseCallBack(std::bind(&CClientRaftImpl::ClientResponseCallBack, this, std::placeholders::_1, std::placeholders::_2));
    _net->SetClientConnectCallBack(std::bind(&CClientRaftImpl::ClientConnect, this, std::placeholders::_1));
    _net->SetClientDisConnectCallBack(std::bind(&CClientRaftImpl::ClientDisConnect, this, std::placeholders::_1));
    _net->SetNodeInfoResponseCallBack(std::bind(&CClientRaftImpl::NodeInfoResponseCallBack, this, std::placeholders::_1, std::placeholders::_2));
}

CClientRaftImpl::~CClientRaftImpl() {

}

void CClientRaftImpl::Init(const std::string& config_file) {
    _config.reset(new CConfigImpl(config_file));

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
        absl::SimpleAtoi<uint32_t>(handle_vec[1], &port);
        _net->ConnectTo(handle_vec[0], (uint16_t)port);
    }
}

void CClientRaftImpl::Dealloc() {
    _net->Dealloc();
}

void CClientRaftImpl::Join() {
    _net->Join();
}

void CClientRaftImpl::SendEntries(const std::string& entries) {
    ClientRequest reqeust;
    reqeust.set_entries(entries);
    _resend_entries = entries;
    _net->SendClientRequest(_cur_net_handle, reqeust);
}

void CClientRaftImpl::SetCommitEntriesCallBack(std::function<void(ERR_CODE)> func) {
    _response_call_back = func;
}

void CClientRaftImpl::ClientConnect(const std::string& net_handle) {
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

void CClientRaftImpl::ClientDisConnect(const std::string& net_handle) {
    _cur_net_handle.clear();
    _net_handle_set.erase(net_handle);
    if (_net_handle_set.size()) {
        auto iter = _net_handle_set.begin();
        std::vector<std::string> handle_vec = absl::StrSplit(*iter, ":");
        if (handle_vec.size() == 2) {
            uint32_t port = 0;
            absl::SimpleAtoi<uint32_t>(handle_vec[1], &port);
            _net->ConnectTo(handle_vec[0], (uint16_t)port);
        }
    }
}

void CClientRaftImpl::ClientResponseCallBack(const std::string& net_handle, ClientResponse& response) {
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
            absl::SimpleAtoi<uint32_t>(handle_vec[1], &port);
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

void CClientRaftImpl::NodeInfoResponseCallBack(const std::string& net_handle, NodeInfoResponse& response) {
    auto size = response.net_handle_size();
    for (int i = 0; i < size; i++) {
        std::string net_handle = response.net_handle(i);
        if (_net_handle_set.count(net_handle) != 0) {
            continue;
        }
        _net_handle_set.insert(std::move(net_handle));
    }
}