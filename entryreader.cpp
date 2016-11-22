#include "entryreader.hpp"
#include <cstring>

EntryReader::EntryReader(std::istream &is)
    : is(is)
    , eof(false)
    , lastTimePos(1)
{
    char c;
    do {
        is >> c;
    } while (c != '[');
    do {
        is >> c;
    } while (isspace(c));
    is.putback(c);
}


double EntryReader::extractTime(const std::string &s) const {
    static constexpr char timeStr[] = "\"time\":";
    static constexpr int timeStrLen = sizeof(timeStr) - 1;

    if (lastTimePos >= s.size() || s.compare(lastTimePos, timeStrLen, timeStr) != 0) {
        auto pos = s.find("\"time\":", 0);
        if (pos == std::string::npos) {
            lastTimePos = 0;
            return -1;
        } else {
            lastTimePos = pos;
        }
    }
    return strtod(s.data() + lastTimePos + timeStrLen, NULL);
}


void EntryReader::operator >>(Entry &ent) {
    if (eof)
        return;
    std::string &s = ent.str;
    std::getline(is, s);
    ent.time = extractTime(s);

    if (s[s.size() - 1] == ',') {
        s.resize(s.size() - 1);
    } else {
        eof = true;
    }
}
