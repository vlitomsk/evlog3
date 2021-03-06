#ifndef ENTRYREADER_HPP_
#define ENTRYREADER_HPP_

#include <iostream>
#include <string>
#include "entry.hpp"
#include "cachedstringfinder.hpp"

class EntryReader {
    std::istream &is;
    bool eof;
    const CachedStringFinder timeFinder;
    double extractTime(const std::string &) const;
public:
    EntryReader(std::istream &is);

    inline bool isEof() const {
        return eof;
    }

    void operator >>(Entry &ent);
};

#endif // ENTRYREADER_HPP_

