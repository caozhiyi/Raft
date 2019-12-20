#ifndef HEADER_CMSG_ROUTER
#define HEADER_CMSG_ROUTER

#include <unordered_map>
#include "common.h"

namespace raft {

    class CNode;
    class CSevManager;
    class CCliManager;

    class CMsgRouter {
    public:
        void Init(CNode* node, CSevManager* sev_manager, CCliManager* cli_manager);

        void HandleHeart(const std::string& ip_port, std::vector<std::string>& msg_vec, long long version, bool done_msg, long long& new_version);

        void HandleVote(const std::string& ip_port, long long version, bool& vote);

        void GetNodeInfo(const std::string& ip_port, std::vector<std::string>& node_info);

        void AddNewNode(const std::string& ip_port, const std::vector<std::string>& node_info);

        void HandleClient(const std::string& ip_port, const std::string& msg, ::raft::ClientResponse* response,
            ::google::protobuf::Closure* done);

    public:
        CNode*          _node;
        CSevManager*    _sev_manager;
        CCliManager*    _cli_manager;
    };
}
#endif