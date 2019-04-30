#include "Tool.h"

std::vector<std::string> SplitStr(std::string str, const std::string& sep) {
    int len = sep.length();
    int pos = 0;
    std::vector<std::string> res;
    std::string temp;
    for (;;) {
        pos = str.find(sep);
        if (pos == std::string::npos) {
            res.push(str);
            break;
        }
        temp = str.substr(0, pos);
        if (!temp.empty()){
            res.push(temp);
        }
        str = str.substr(pos + len, sep.length());
    }
    return std::move(res);
}
