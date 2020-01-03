#include <functional>

#include "INet.h"
#include "IRole.h"
#include "IClient.h"
#include "message.pb.h"
#include "MountClient.h"


using namespace raft;

CMountClient::CMountClient(std::shared_ptr<CNet>& net) : _net(net) {
    _net->SetClientResponseCallBack(std::bind(&CMountClient::ClientResponseCallBack, this, std::placeholders::_1, std::placeholders::_2));
}

CMountClient::~CMountClient() {

}

void CMountClient::SetLeaderHandle(const std::string& net_handle) {
    _cur_net_handle = net_handle;
    if (!_resend_entries.empty()) {
        SendEntries(_resend_entries);
    }
}

void CMountClient::SetCurRole(std::shared_ptr<CRole>& role) {
    _current_role = role;
}

void CMountClient::SendEntries(const std::string& entries) {
    // current node is leader
    ClientRequest reqeust;
    reqeust.set_entries(entries);
    _resend_entries = entries;
    if (_current_role && _current_role->GetRole() == leader_role) {
        std::shared_ptr<CClient> client(nullptr);
        _current_role->RecvClientRequest(client, reqeust);

    } else {
        if (!_cur_net_handle.empty()) {
            _net->SendClientRequest(_cur_net_handle, reqeust);
        }
        // else, do nothing, wait a leader heart
    }
}

void CMountClient::ClientResponseCallBack(const std::string& net_handle, ClientResponse& response) {
    auto ret_code = response.ret_code();
    if (ret_code == success) {
        _resend_entries.clear();
    
    } else if (ret_code == not_leader) {
        // do nothing, current node wait heart from leader

    } else if (ret_code == other_error) {
        // do nothing, current node wait heart from leader
        
    } else if (ret_code == send_again) {
        // do nothing, current node wait heart from leader
    }
}