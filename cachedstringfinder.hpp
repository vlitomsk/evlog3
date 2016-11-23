#ifndef CACHEDSTRINGFINDER_HPP_
#define CACHEDSTRINGFINDER_HPP_

#include <string>

class CachedStringFinder {
    mutable size_t lastPos;
    const std::string substr;
    const size_t substrLen;
public:
    CachedStringFinder(const std::string &substr, size_t initialFindPos = 0);

    inline size_t getSubstrLen() const {
        return substrLen;
    }

    size_t findIn(const std::string &s) const;
};

#endif // CACHEDSTRINGFINDER_HPP_

