#ifndef RAFT_EXAMPLE_NODESERVER_SERVER
#define RAFT_EXAMPLE_NODESERVER_SERVER

#include <map>
#include <mutex>
#include <string>

#include "Raft.h"
#include "HttpServerImpl.h"

class CServer {
public:
    // start raft and http
    void Init(const std::string& config_file);

    void Join();

    // get request from http client
    std::string DoRequest(RequestType type, const std::string& key, const std::string& value);

    // get entries from raft
    void RecvEntries(std::string entries);
    
private:
    std::mutex                         _mutex;
    // all key value cache
    std::map<std::string, std::string> _data_map;
    // http server 
    HttpServerImpl                     _http_server;
};

#endif