#include "entryreader.hpp"
#include <cstring>

EntryReader::EntryReader(std::istream &is)
    : is(is)
    , eof(false)
    , timeFinder("\"time\":", 1)
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
    auto timePos = timeFinder.findIn(s);
    return timePos == std::string::npos
            ? -1
            : strtod(s.data() + timePos + timeFinder.getSubstrLen(), NULL);
}


void EntryReader::operator >>(Entry &ent) {
    if (eof)
        return;

    std::string s;
    std::getline(is, s);
    if (s[s.size() - 1] == ',') {
        s.resize(s.size() - 1);
    } else {
        eof = true;
    }
    ent.setStr(s);
}
