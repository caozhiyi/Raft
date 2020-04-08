#ifndef RAFT_CPMMON_CPPNETIMPLWITHOUTCLIENT
#define RAFT_CPMMON_CPPNETIMPLWITHOUTCLIENT

#include <map>
#include <functional>

#include "INet.h"
#include "CppNet.h"
#include "CppDefine.h"
#include "NetCommon.h"
namespace raft {
    
    class CNode;
    class CCppNetImpl : public CNet {
    public:
        CCppNetImpl(std::shared_ptr<CNodeManager> node_manager);
        ~CCppNetImpl();

        void Init(uint16_t thread_num);
        // start to listen
        bool Start(const std::string& ip, uint16_t port);
        void Join();
        void Dealloc();
        // connect to
        void ConnectTo(const std::string& ip, uint16_t port);
        void DisConnect(const std::string& net_handle);

        // node info
        void SendNodeInfoRequest(const std::string& net_handle, NodeInfoRequest& request);
        void SendNodeInfoResponse(const std::string& net_handle, NodeInfoResponse& response);
        // heart beat
        void SendHeartRequest(const std::string& net_handle, HeartBeatResquest& request);
        void SendHeartResponse(const std::string& net_handle, HeartBeatResponse& response);
        // vote
        void SendVoteRequest(const std::string& net_handle, VoteRequest& request);
        void SendVoteResponse(const std::string& net_handle, VoteResponse& response);

        // client about
        void SendEntriesRequest(const std::string& net_handle, EntriesRequest& request);
        void SendEntriesResponse(const std::string& net_handle, EntriesResponse& response);

    private:
        struct CppBag {
            union Header {
                struct Field {
                    uint32_t  _len;
                    uint32_t  _type;
                } _field;
                uint64_t      _data;
            } _header;
            std::string       _body;
        };
        // cppnet call back
        void Connected(const cppnet::Handle& handle, uint32_t err);
        void DisConnected(const cppnet::Handle& handle, uint32_t err);
        void Sended(const cppnet::Handle& handle, uint32_t len, uint32_t err);
        void Recved(const cppnet::Handle& handle, base::CBuffer* data, uint32_t len, uint32_t err);
        // bag about
        void BuildSendData(std::string& data, MessageType type, std::string& ret);
        bool StringToBag(const std::string& data, std::vector<CppBag>& bag_vec, uint32_t& used_size);
        // get net handle
        std::string GetNetHandle(const cppnet::Handle& handle);
        // send to net
        void SendToNet(const std::string& net_handle, std::string& data);
        // handle bag
        void HandleBag(const std::string& net_handle, const CppBag& bag);

    private:
        // net handle to cppnet handle
        std::map<std::string, cppnet::Handle>                                 _net_2_handle_map;
        std::map<cppnet::Handle, std::string>                                 _handle_2_net_map;
    };
}

#endif