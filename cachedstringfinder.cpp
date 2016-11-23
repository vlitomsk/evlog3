#include "cachedstringfinder.hpp"

CachedStringFinder::
CachedStringFinder(const std::string &substr, size_t initialFindPos)
    : lastPos(initialFindPos)
    , substr(substr)
    , substrLen(substr.length())
{}

size_t CachedStringFinder::findIn(const std::string &s) const {
    if (lastPos >= s.size() || s.compare(lastPos, substrLen, substr) != 0) {
        auto pos = s.find(substr, 0);
        if (pos == std::string::npos) {
            lastPos = 0;
            return std::string::npos;
        } else {
            lastPos = pos;
        }
    }
    return lastPos;
}
