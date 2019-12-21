#include "absl/strings/numbers.h"
#include "Log.h"
#include "Socket.h"
#include "CppnetImpl.h"

using namespace raft;

CCppNet::CCppNet() {

}

CCppNet::~CCppNet() {

}

bool CCppNet::Start(const std::string& ip, uint16_t port) {
    // set call back to cppnet
    cppnet::SetReadCallback(std::bind(&CCppNet::Recved, this, std::placeholders::_1, std::placeholders::_2, 
                                              std::placeholders::_3, std::placeholders::_4));
    cppnet::SetWriteCallback(std::bind(&CCppNet::Sended, this, std::placeholders::_1, std::placeholders::_2, 
                                              std::placeholders::_3));
    cppnet::SetDisconnectionCallback(std::bind(&CCppNet::DisConnected, this, std::placeholders::_1, std::placeholders::_2));
    cppnet::SetAcceptCallback(std::bind(&CCppNet::Connected, this, std::placeholders::_1, std::placeholders::_2));
    cppnet::SetConnectionCallback(std::bind(&CCppNet::Connected, this, std::placeholders::_1, std::placeholders::_2));

    // start cpp net
    cppnet::Init(1); // start with 1 thread
    return cppnet::ListenAndAccept(ip, port);
}

void CCppNet::Join() {
    cppnet::Join();
}

void CCppNet::ConnectTo(const std::string& ip, uint16_t port) {
    cppnet::Connection(ip, port);
}

void CCppNet::SendHeartRequest(const std::string& net_handle, HeartBeatResquest& request) {
    std::string data;
    request.SerializeToString(&data);

    SendToNet(net_handle, data);
}

void CCppNet::SendHeartResponse(const std::string& net_handle, HeartBeatResponse& response) {
    std::string data;
    response.SerializeToString(&data);

    SendToNet(net_handle, data);
}

void CCppNet::SendVoteRequest(const std::string& net_handle, VoteRequest& request) {
    std::string data;
    request.SerializeToString(&data);

    SendToNet(net_handle, data);
}

void CCppNet::SendVoteResponse(const std::string& net_handle, VoteResponse& response) {
    std::string data;
    response.SerializeToString(&data);

    SendToNet(net_handle, data);
}

void CCppNet::SendToClient(const std::string& net_handle, ClientResponse& response) {
    std::string data;
    response.SerializeToString(&data);

    SendToNet(net_handle, data, user_client);
}

void CCppNet::SetClientRecvCallBack(absl::FunctionRef<void(const std::string&, ClientRequest&)> func) {
    _client_recv_call_back = func;
}

void CCppNet::SetClientConnectCallBack(absl::FunctionRef<void(const std::string&)> func) {
    _client_connect_call_back = func;
}

void CCppNet::SetClientDisConnectCallBack(absl::FunctionRef<void(const std::string&)> func) {
    _client_dis_connect_call_back = func;
}

void CCppNet::SetNewConnectCallBack(absl::FunctionRef<void(const std::string&)> func) {
    _raft_connect_call_back = func;
}

void CCppNet::SetDisConnectCallBack(absl::FunctionRef<void(const std::string&)> func) {
    _raft_dis_connect_call_back = func;
}

void CCppNet::SetHeartRequestRecvCallBack(absl::FunctionRef<void(const std::string&, HeartBeatResquest&)> func) {
    _heart_request_call_back = func;
}

void CCppNet::SetHeartResponseRecvCallBack(absl::FunctionRef<void(const std::string&, HeartBeatResponse&)> func) {
    _heart_response_call_back = func;
}

void CCppNet::SetVoteRequestRecvCallBack(absl::FunctionRef<void(const std::string&, VoteRequest&)> func) {
    _vote_request_call_back = func;
}

void CCppNet::SetVoteResponseRecvCallBack(absl::FunctionRef<void(const std::string&, VoteResponse&)> func) {
    _vote_response_call_back = func;
}

void CCppNet::Connected(const cppnet::Handle& handle, uint32_t err) {
    if (err != cppnet::CEC_SUCCESS) {
        base::LOG_ERROR("some thing wrong while connect, err : %d", err);
        return;
    }

    auto net_handle = GetNetHandle(handle);

    _net_2_handle_map[net_handle.first] = handle;
    _handle_2_net_map[handle] = net_handle;
}

void CCppNet::DisConnected(const cppnet::Handle& handle, uint32_t err) {

    auto net_handle = GetNetHandle(handle);
    
    // is client?
    if (net_handle.second == user_client) {
        _client_dis_connect_call_back(net_handle.first);

    // raft message
    } else if (net_handle.second == raft_node) {
        _raft_dis_connect_call_back(net_handle.first);
    
    } else {
        base::LOG_WARN("unkonw connection lost.");
    }

    _net_2_handle_map.erase(net_handle.first);
    _handle_2_net_map.erase(handle);
}

void CCppNet::Sended(const cppnet::Handle& handle, uint32_t len, uint32_t err) {
    if (err != cppnet::CEC_SUCCESS) {
        DisConnected(handle, err);
    }
}

void CCppNet::Recved(const cppnet::Handle& handle, base::CBuffer* data, uint32_t len, uint32_t err) {
    if (err != cppnet::CEC_SUCCESS) {
        DisConnected(handle, err);
        return;
    }

    uint32_t total_len = data->GetCanReadLength();
    if (total_len <= __header_len) {
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
    if (net_handle.first.empty()) {
        base::LOG_ERROR("can't get net handle when recv data.");
        delete []buf;
        return;
    }

    // check connect type
    if (net_handle.second == unknow_type) {
        CheckConnectType(handle, net_handle.first, (CppBagType)bag_vec[0]._header._field._type);
    }

    // handle message
    for(auto iter : bag_vec) {
        HandleBag(net_handle.first, iter);
    }

    delete []buf;
}

std::string CCppNet::BagToString(CppBag& bag) {
    std::string ret;
    ret.append(std::to_string(bag._header._data));
    ret.append(bag._body);
    return std::move(ret);
}

bool CCppNet::StringToBag(const std::string& data, std::vector<CppBag>& bag_vec, uint32_t& used_size) {
    char* start = (char*)data.c_str();
    uint32_t offset = 0;
    uint32_t left_len = data.length();
    bool ret = false;
    while(true) {
        CppBag bag;
        std::string header(start + offset, __header_len);
        absl::SimpleAtoi<uint64_t>(header, &bag._header._data);

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

std::pair<std::string, ClientType> CCppNet::GetNetHandle(const cppnet::Handle& handle) {
    std::pair<std::string, ClientType> ret;
    auto iter = _handle_2_net_map.find(handle);
    if (iter != _handle_2_net_map.end()) {
        ret.first = iter->second.first;
        ret.second = iter->second.second;

    } else {
        uint16_t port;
        if(cppnet::GetIpAddress(handle, ret.first, port) == cppnet::CEC_SUCCESS) {
            ret.first.append(":");
            ret.first.append(std::to_string(port));
        }
        ret.second = unknow_type;
    }
    return std::move(ret);
}

void CCppNet::SendToNet(const std::string& net_handle, std::string& data, ClientType type) {
    auto iter = _net_2_handle_map.find(net_handle);
    if (iter == _net_2_handle_map.end()) {
        if (type == raft_node) {
            _raft_dis_connect_call_back(net_handle);
            
        } else {
            _client_dis_connect_call_back(net_handle);
        }
        return;
    }
    
    cppnet::Write(iter->second, data.c_str(), data.length());
}

void CCppNet::HandleBag(const std::string& net_handle, const CppBag& bag) {
    switch (bag._header._field._type)
    {
    case heart_beat_request:
        {
            HeartBeatResquest request;
            request.ParseFromString(bag._body);
            _heart_request_call_back(net_handle, request);
            break;
        }
    case heart_beat_response:
        {
            HeartBeatResponse response;
            response.ParseFromString(bag._body);
            _heart_response_call_back(net_handle, response);
            break;
        }
    case vote_request:
        {
            VoteRequest request;
            request.ParseFromString(bag._body);
            _vote_request_call_back(net_handle, request);
            break;
        }
    case vote_reponse:
        {
            VoteResponse response;
            response.ParseFromString(bag._body);
            _vote_response_call_back(net_handle, response);
            break;
        }
    case client_type:
        {
            ClientRequest request;
            request.ParseFromString(bag._body);
            _client_recv_call_back(net_handle, request);
            break;
        }
    default:
        base::LOG_ERROR("unknow type while handle bag");
        break;
    }
}

void CCppNet::CheckConnectType(const cppnet::Handle& handle, const std::string& net_handle, CppBagType type) {
    ClientType client_type = unknow_type;
    if (type == client_type) {
        client_type = user_client;
        _client_connect_call_back(net_handle);

    } else {
        client_type = raft_node;
        _raft_connect_call_back(net_handle);
    }

    _handle_2_net_map[handle].second = client_type;
}