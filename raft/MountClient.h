#ifndef RAFT_RAFT_MOUNTCLIENT
#define RAFT_RAFT_MOUNTCLIENT

#include "IClient.h"
#include "message.pb.h"

namespace raft {

    class CNet;
    class CRole;
    class CMountClient {
    public:
        CMountClient(std::shared_ptr<CNet>& net);
        ~CMountClient();
        
        // set currnet leader net handle
        void SetLeaderHandle(const std::string& net_handle);
        // set current role
        void SetCurRole(std::shared_ptr<CRole>& role);

        // send entries
        void SendEntries(const std::string& entries);
    
        // client response call back
        void ClientResponseCallBack(const std::string& net_handle, ClientResponse& response);
    private:
        std::shared_ptr<CNet>           _net;
        std::shared_ptr<CRole>          _current_role;
        std::string                     _cur_net_handle;
        std::string                     _resend_entries;
    };
}

#endif