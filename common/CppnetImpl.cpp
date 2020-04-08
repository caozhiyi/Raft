#include "Log.h"
#include "Socket.h"
#include "CppnetImpl.h"
#include "INodeManager.h"
#include "absl/strings/numbers.h"

using namespace raft;

static const uint32_t __header_len = sizeof(uint64_t);

CCppNetImpl::CCppNetImpl(std::shared_ptr<CNodeManager> node_manager) : CNet(node_manager) {

}

CCppNetImpl::~CCppNetImpl() {

}

void CCppNetImpl::Init(uint16_t thread_num) {
    // start cpp net
    uint32_t net_handle = cppnet::Init(thread_num);

    // set call back to cppnet
    cppnet::SetReadCallback(std::bind(&CCppNetImpl::Recved, this, std::placeholders::_1, std::placeholders::_2, 
                                              std::placeholders::_3, std::placeholders::_4), net_handle);
    cppnet::SetWriteCallback(std::bind(&CCppNetImpl::Sended, this, std::placeholders::_1, std::placeholders::_2, 
                                              std::placeholders::_3), net_handle);
    cppnet::SetDisconnectionCallback(std::bind(&CCppNetImpl::DisConnected, this, std::placeholders::_1, std::placeholders::_2), net_handle);
    cppnet::SetAcceptCallback(std::bind(&CCppNetImpl::Connected, this, std::placeholders::_1, std::placeholders::_2), net_handle);
    cppnet::SetConnectionCallback(std::bind(&CCppNetImpl::Connected, this, std::placeholders::_1, std::placeholders::_2), net_handle);
}

bool CCppNetImpl::Start(const std::string& ip, uint16_t port) {
    return cppnet::ListenAndAccept(ip, port);
}

void CCppNetImpl::Join() {
    cppnet::Join();
}

void CCppNetImpl::Dealloc() {
    cppnet::Dealloc();
}

void CCppNetImpl::ConnectTo(const std::string& ip, uint16_t port) {
    cppnet::Connection(ip, port);
}

void CCppNetImpl::DisConnect(const std::string& net_handle) {
    auto iter = _net_2_handle_map.find(net_handle);
    if (iter == _net_2_handle_map.end()) {
        return;
    }
    cppnet::Close(iter->second);
}

void CCppNetImpl::SendNodeInfoRequest(const std::string& net_handle, NodeInfoRequest& request) {
    std::string data;
    request.SerializeToString(&data);

    std::string send_data;
    BuildSendData(data, node_info_request, send_data);

    SendToNet(net_handle, send_data);
}

void CCppNetImpl::SendNodeInfoResponse(const std::string& net_handle, NodeInfoResponse& response) {
    std::string data;
    response.SerializeToString(&data);

    std::string send_data;
    BuildSendData(data, node_info_response, send_data);

    SendToNet(net_handle, send_data);
}

void CCppNetImpl::SendHeartRequest(const std::string& net_handle, HeartBeatResquest& request) {
    std::string data;
    request.SerializeToString(&data);

    std::string send_data;
    BuildSendData(data, heart_beat_request, send_data);

    SendToNet(net_handle, send_data);
}

void CCppNetImpl::SendHeartResponse(const std::string& net_handle, HeartBeatResponse& response) {
    std::string data;
    response.SerializeToString(&data);

    std::string send_data;
    BuildSendData(data, heart_beat_response, send_data);

    SendToNet(net_handle, send_data);
}

void CCppNetImpl::SendVoteRequest(const std::string& net_handle, VoteRequest& request) {
    std::string data;
    request.SerializeToString(&data);

    std::string send_data;
    BuildSendData(data, vote_request, send_data);

    SendToNet(net_handle, send_data);
}

void CCppNetImpl::SendVoteResponse(const std::string& net_handle, VoteResponse& response) {
    std::string data;
    response.SerializeToString(&data);

    std::string send_data;
    BuildSendData(data, vote_response, send_data);

    SendToNet(net_handle, send_data);
}

void CCppNetImpl::SendEntriesRequest(const std::string& net_handle, EntriesRequest& request) {
    std::string data;
    request.SerializeToString(&data);

    std::string send_data;
    BuildSendData(data, entries_requst, send_data);

    SendToNet(net_handle, send_data);
}

void CCppNetImpl::SendEntriesResponse(const std::string& net_handle, EntriesResponse& response) {
    std::string data;
    response.SerializeToString(&data);

    std::string send_data;
    BuildSendData(data, entries_response, send_data);

    SendToNet(net_handle, send_data);
}

void CCppNetImpl::Connected(const cppnet::Handle& handle, uint32_t err) {
    if (err != cppnet::CEC_SUCCESS) {
        base::LOG_ERROR("some thing wrong while connect, err : %d", err);
        return;
    }

    auto net_handle = GetNetHandle(handle);

    _net_2_handle_map[net_handle] = handle;
    _handle_2_net_map[handle] = net_handle;

    _node_manager->NewConnectCallBack(net_handle);
}

void CCppNetImpl::DisConnected(const cppnet::Handle& handle, uint32_t err) {

    auto net_handle = GetNetHandle(handle);

    _node_manager->DisConnectCallBack(net_handle);
  
    _net_2_handle_map.erase(net_handle);
    _handle_2_net_map.erase(handle);
}

void CCppNetImpl::Sended(const cppnet::Handle& handle, uint32_t len, uint32_t err) {
    if (err != cppnet::CEC_SUCCESS) {
        DisConnected(handle, err);
    }
}

void CCppNetImpl::Recved(const cppnet::Handle& handle, base::CBuffer* data, uint32_t len, uint32_t err) {
    if (err != cppnet::CEC_SUCCESS) {
        DisConnected(handle, err);
        return;
    }

    uint32_t total_len = data->GetCanReadLength();
    if (total_len < __header_len) {
        return;
    }
    
    // parse data
    char* buf = new char[total_len];

    uint32_t get_len = data->ReadNotClear(buf, total_len);
    std::vector<CppBag> bag_vec;
    uint32_t used_size = 0;
    if (!StringToBag(std::string(buf, get_len), bag_vec, used_size)) {
        delete []buf;
        return;
    }
    data->Clear(used_size);

    auto net_handle = GetNetHandle(handle);
    if (net_handle.empty()) {
        base::LOG_ERROR("can't get net handle when recv data.");
        delete []buf;
        return;
    }
    // handle message
    for(auto iter : bag_vec) {
        HandleBag(net_handle, iter);
    }

    delete []buf;
}

void CCppNetImpl::BuildSendData(std::string& data, MessageType type, std::string& ret) {
    CppBag bag;
    bag._header._field._len = data.length();
    bag._header._field._type = type;
    
    int  size = sizeof(bag._header._data);
    char header_buf[__header_len];
    memcpy(header_buf, &bag._header._data, __header_len);
    ret.append(header_buf, __header_len);
    ret.append(data);
}

bool CCppNetImpl::StringToBag(const std::string& data, std::vector<CppBag>& bag_vec, uint32_t& used_size) {
    char* start = (char*)data.c_str();
    uint32_t offset = 0;
    uint32_t left_len = data.length();
    bool ret = false;
    while(true) {
        CppBag bag;
        memcpy(&bag._header._data, start + offset, __header_len);

        if (data.length() >= bag._header._field._len + __header_len) {
            bag._body = std::string(data.data() + __header_len, bag._header._field._len);

            ret = true;
            bag_vec.push_back(std::move(bag));

            offset += __header_len + bag._header._field._len;
            left_len -= __header_len + bag._header._field._len;
            if (left_len <= __header_len) {
                break;
            }

        } else {
            break;
        }
    }
    used_size = offset;
    return ret;
}

std::string CCppNetImpl::GetNetHandle(const cppnet::Handle& handle) {
    auto iter = _handle_2_net_map.find(handle);
    if (iter != _handle_2_net_map.end()) {
        return iter->second;

    } else {
        std::string ip;
        uint16_t port;
        if(cppnet::GetIpAddress(handle, ip, port) == cppnet::CEC_SUCCESS) {
            ip.append(":");
            ip.append(std::to_string(port));
        }
        return std::move(ip);
    }
}

void CCppNetImpl::SendToNet(const std::string& net_handle, std::string& data) {
    auto iter = _net_2_handle_map.find(net_handle);
    if (iter == _net_2_handle_map.end()) {
        _node_manager->DisConnectCallBack(net_handle);
        return;
    }
    
    cppnet::Write(iter->second, data.c_str(), data.length());
}

void CCppNetImpl::HandleBag(const std::string& net_handle, const CppBag& bag) {
    switch (bag._header._field._type)
    {
    case heart_beat_request:
        {
            HeartBeatResquest request;
            request.ParseFromString(bag._body);
            _node_manager->HeartRequestRecvCallBack(net_handle, request);
            break;
        }
    case heart_beat_response:
        {
            HeartBeatResponse response;
            response.ParseFromString(bag._body);
            _node_manager->HeartResponseRecvCallBack(net_handle, response);
            break;
        }
    case vote_request:
        {
            VoteRequest request;
            request.ParseFromString(bag._body);
            _node_manager->VoteRequestRecvCallBack(net_handle, request);
            break;
        }
    case vote_response:
        {
            VoteResponse response;
            response.ParseFromString(bag._body);
            _node_manager->VoteResponseRecvCallBack(net_handle, response);
            break;
        }
    case entries_requst:
        {
            EntriesRequest request;
            request.ParseFromString(bag._body);
            _node_manager->EntriesRequestCallBack(net_handle, request);
            break;
        }
    case entries_response:
        {
            EntriesResponse response;
            response.ParseFromString(bag._body);
            _node_manager->EntriesResponseCallBack(net_handle, response);
            break;
        }
    case node_info_request:
        {
            NodeInfoRequest request;
            request.ParseFromString(bag._body);
            _node_manager->NodeInfoRequestCallBack(net_handle, request);
            break;
        }
    case node_info_response:
        {
            NodeInfoResponse response;
            response.ParseFromString(bag._body);
            _node_manager->NodeInfoResponseCallBack(net_handle, response);
            break;
        }
    default:
        base::LOG_ERROR("unknow type while handle bag");
        break;
    }
}