#ifndef HEADER_LOG_REPLICATION
#define HEADER_LOG_REPLICATION

#include <string>
#include <vector>
#include <fstream>

#include "base/RunnableAloneTaskList.h"
#include "base/Single.h"
#include "common.h"

namespace raft {
    typedef std::pair<Time, std::string> BinLog;
    class CBinLog : public CRunnableAloneTaskList<BinLog> {
    public:
        CBinLog(std::string fine_name = "raft_bin_log.log");
        ~CBinLog();

        //thread
        void Run();
        void Stop();

        //submit log
        bool PushLog(const std::string& log);
        //submit log with time
        void PushLog(Time time, std::string log);
        //get log vector after time
        bool GetLog(Time time, std::vector<BinLog>& log_vec);
        //get the newest time
        Time GetNewestTime();
        //get now utc time
        Time GetUTC();

    public:
        //get the newer log
        std::vector<std::string> GetTargetLogStr(int count);
        //parser log str to BinLog
        BinLog StrToBinLog(const std::string& log);
        //parser BinLog to log str
        std::string BinLogToStr(const BinLog& log);

    private:
        std::string        _file_name;
        std::fstream       _in_file_stream;
        std::fstream       _out_file_stream;
        std::atomic<Time>  _newest_time;
        std::mutex		   _mutex;
    };
}

#endif