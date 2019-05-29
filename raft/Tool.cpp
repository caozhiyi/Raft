#include "Tool.h"
namespace raft {
    std::vector<std::string> SplitStr(std::string str, const std::string& sep) {
        int len = sep.length();
        int pos = 0;
        std::vector<std::string> res;
        std::string temp;
        for (;;) {
            pos = str.find(sep);
            if (pos == std::string::npos) {
                res.push_back(str);
                break;
            }
            temp = str.substr(0, pos);
            if (!temp.empty()) {
                res.push_back(temp);
            }
            str = str.substr(pos + len, str.length());
        }
        return std::move(res);
    }
}
