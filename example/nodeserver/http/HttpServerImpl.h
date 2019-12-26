#ifndef TEST_HTTP_HTTPSERVERIMPL_HEADER
#define TEST_HTTP_HTTPSERVERIMPL_HEADER

#include <functional>
#include "HttpResponse.h"
#include "HttpServer.h"
#include "HttpRequest.h"

enum RequestType {
    request_add    = 1,
    request_remove = 2,
    request_modify = 3,
    request_query  = 4
};

class HttpServerImpl {
public:
    // init cppnet about
    void Init(const std::string& ip, int port);
    void Join();
    // recv request from client
    void RecvRequest(const CHttpRequest& req, CHttpResponse& resp);

    // disconnect from client
    void DisConnect(const cppnet::Handle& , uint32_t);

    void SetRequestCallBack(const std::function<std::string(RequestType type, const std::string& key, const std::string& value)>& func);

private:
    CHttpServer _server;
    std::function<std::string(RequestType type, const std::string& key, const std::string& value)> _request_call_back;
};

#endif