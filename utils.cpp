#include "utils.h"

size_t split(const string &txt, vector<string> &strs, char ch) {
    size_t pos = txt.find(ch);
    size_t initialPos = 0;
    strs.clear();
    while (pos != string::npos) {
        strs.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;
        pos = txt.find(ch, initialPos);
    }
    strs.push_back(txt.substr(initialPos, min(pos, txt.size()) - initialPos + 1));
    return strs.size();
}
