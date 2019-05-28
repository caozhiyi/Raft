#include <iostream>
#include <functional>
#include "CClient.h"
#include "config.h"
#include "Log.h"

using namespace std;
void ResponseFunc(int status, const std::string& err) {
    cout << "status: " << status << endl;
    cout << "err   : " << err << endl;
}

int main() {
    CLog::Instance().SetLogName("server.log");
    CLog::Instance().SetLogLevel(LOG_DEBUG_LEVEL);
    CLog::Instance().Start();

    raft::CClient client;
    client.SetCallBackFunc(ResponseFunc);
    client.Init("client.conf");

    while (1) {
        string msg;
        cin >> msg;
        client.SendMsg(msg);
    }
}