#ifndef STATCOLLECTORS_HPP_
#define STATCOLLECTORS_HPP_

#include "consumer.hpp"
#include "prefilters.hpp"

namespace stats {

/// based on "dadd", "drem" events.
class DfMemUsage : public Consumer {

    typedef enum {DADD, DREM, NOTAG} TagType;
    TagType getTagType(const Entry &ent) {
        size_t pos = tagFinder.findIn(ent.str);
        if (pos == std::string::npos)
            return NOTAG;
        pos += tagFinder.getSubstrLen() + 1;
        if (ent.str.compare(pos, 4, "dadd") == 0)
            return DADD;
        else if (ent.str.compare(pos, 4, "drem") == 0)
            return DREM;
        else
            return NOTAG;
    }

    ssize_t getSize(const Entry &ent) {
        size_t pos = sizeFinder.findIn(ent.str);
        if (pos == std::string::npos)
            return std::numeric_limits<int>::max();
        return strtol(ent.str.data() + pos + sizeFinder.getSubstrLen(), NULL, 10);
    }

    CachedStringFinder tagFinder;
    CachedStringFinder sizeFinder;
    ssize_t usage;
    std::function<void(double,ssize_t)> storeFn;
public:
    DfMemUsage(std::function<void(double,ssize_t)> storeFn)
        : tagFinder("\"tag\":")
        , sizeFinder("\"size\":")
        , storeFn(storeFn)
    {}

    virtual void operator <<(const Entry &ent) override {
        TagType tt = getTagType(ent);
        switch (tt) {
            case NOTAG: return;
            case DADD: usage += getSize(ent); break;
            case DREM: usage -= getSize(ent); break;
        }
        storeFn(ent.time, usage);
    }
};

class N

};

#endif // STATCOLLECTORS_HPP_

