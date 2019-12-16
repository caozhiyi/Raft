#include "CppnetImpl.h"
#include "absl/strings/numbers.h"
#include "Log.h"
#include "Socket.h"

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

void CCppNet::SetNewConnectCallBack(absl::FunctionRef<void(absl::string_view net_handle)> func) {
    _new_connect_call_back = func;
}

void CCppNet::SetDisConnectCallBack(absl::FunctionRef<void(absl::string_view net_handle)> func) {
    _dis_connect_call_back = func;
}

void CCppNet::SetHeartRequestRecvCallBack(absl::FunctionRef<void(absl::string_view, HeartBeatResquest&)> func) {
    _heart_request_call_back = func;
}

void CCppNet::SetHeartResponseRecvCallBack(absl::FunctionRef<void(absl::string_view, HeartBeatResponse&)> func) {
    _heart_response_call_back = func;
}

void CCppNet::SetVoteRequestRecvCallBack(absl::FunctionRef<void(absl::string_view, VoteRequest&)> func) {
    _vote_request_call_back = func;
}

void CCppNet::SetVoteResponseRecvCallBack(absl::FunctionRef<void(absl::string_view, VoteResponse&)> func) {
    _vote_response_call_back = func;
}

void CCppNet::Connected(const cppnet::Handle& handle, uint32_t err) {
    if (err != cppnet::CEC_SUCCESS) {
        base::LOG_ERROR("some thing wrong while connect, err : %d", err);
        return;
    }

    std::string net_handle = GetNetHandle(handle);
    _new_connect_call_back(net_handle);

    _net_2_handle_map[net_handle] = handle;
    _handle_2_net_map[handle] = net_handle;
}

void CCppNet::DisConnected(const cppnet::Handle& handle, uint32_t err) {

    std::string net_handle = GetNetHandle(handle);
    
    _dis_connect_call_back(net_handle);

    _net_2_handle_map.erase(net_handle);
    _handle_2_net_map.erase(handle);
}

void CCppNet::Sended(const cppnet::Handle& handle, uint32_t len, uint32_t err) {
    if (err != cppnet::CEC_SUCCESS) {
        DisConnected(handle, err);
    }
}

void CCppNet::Recved(const  cppnet::Handle& handle, base::CBuffer* data, uint32_t len, uint32_t err) {
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

    uint32_t get_len = data->Read(buf, total_len);
    std::vector<CppBag> bag_vec;
    if (!StringToBag(std::string(buf, get_len), bag_vec)) {
        return;
    }
    std::string net_handle = GetNetHandle(handle);
    if (net_handle.empty()) {
        base::LOG_ERROR("can't get net handle when recv data.");
        return;
    }
    for(auto iter : bag_vec) {
        HandleBag(net_handle, iter);
    }

    delete []buf;
}

std::string CCppNet::BagToString(CppBag& bag) {
    std::string ret;
    ret.append(std::to_string(bag._header._data));
    ret.append(bag._body);
    return std::move(ret);
}

bool CCppNet::StringToBag(const std::string& data, std::vector<CppBag>& bag_vec) {
    char* start = (char*)data.c_str();
    uint32_t offset = 0;
    uint32_t left_len = data.length();
    bool ret = false;
    while(true) {
        CppBag bag;
        absl::string_view header(start + offset, __header_len);
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
    return ret;
}

std::string CCppNet::GetNetHandle(const cppnet::Handle& handle) {
    std::string net_handle;
    auto iter = _handle_2_net_map.find(handle);
    if (iter != _handle_2_net_map.end()) {
        net_handle = iter->second;

    } else {
        uint16_t port;
        if(cppnet::GetIpAddress(handle, net_handle, port) == cppnet::CEC_SUCCESS) {
            net_handle.append(":");
            net_handle.append(std::to_string(port));
        }
    }
    return std::move(net_handle);
}

void CCppNet::SendToNet(const std::string& net_handle, std::string& data) {
    auto iter = _net_2_handle_map.find(net_handle);
    if (iter == _net_2_handle_map.end()) {
        _dis_connect_call_back(net_handle);
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
    default:
        base::LOG_ERROR("unknow type while handle bag");
        break;
    }
}