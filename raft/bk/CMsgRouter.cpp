#include <algorithm>
#include "CNode.h"
#include "CSevManager.h"
#include "CCliManager.h"

using namespace raft;

void CMsgRouter::Init(CNode* node, CSevManager* sev_manager, CCliManager* cli_manager) {
    _node = node;
    _cli_manager = cli_manager;
    _sev_manager = sev_manager;
}

void CMsgRouter::HandleHeart(const std::string& ip_port, std::vector<std::string>& msg_vec, long long version, bool done_msg, long long& new_version) {
    _node->HandleHeart(ip_port, msg_vec, version, done_msg, new_version);
}

void CMsgRouter::HandleVote(const std::string& ip_port, long long version, bool& vote) {
    _node->HandleVote(ip_port, version, vote);
}

void CMsgRouter::GetNodeInfo(const std::string& ip_port, std::vector<std::string>& node_info) {
    _sev_manager->GetNodeInfo(ip_port, node_info);
}

void CMsgRouter::AddNewNode(const std::string& ip_port, const std::vector<std::string>& node_info) {
    _sev_manager->AddNewNode(ip_port, node_info);
}

void CMsgRouter::HandleClient(const std::string& ip_port, const std::string& msg, ::raft::ClientResponse* response,
    ::google::protobuf::Closure* done) {
    _cli_manager->HandleClient(ip_port, msg, response, done);
}