#ifndef STATCOLLECTORS_HPP_
#define STATCOLLECTORS_HPP_

#include "consumer.hpp"
#include "prefilters.hpp"

namespace stats {

class TimeHistCollector : public Consumer {
protected:
    virtual void flushBin(double binStartTime) = 0;
    virtual void accumulateBin(const Entry &ent) = 0;

private:
    const double dtBinSize;
    double lastFlushTime, lastEntryTime;

    inline void doFlush() {
        flushBin(lastFlushTime);
        lastFlushTime = lastEntryTime;
    }

    inline void onBinOverflow(const Entry &ent) {
        doFlush();
        onBinUnderflow(ent);
    }

    inline void onBinUnderflow(const Entry &ent) {
        accumulateBin(ent);
        lastEntryTime = ent.time;
    }

public:
    TimeHistCollector(double dtBinSize, double initialTime = 0.0)
        : dtBinSize(dtBinSize)
        , lastFlushTime(initialTime)
        , lastEntryTime(initialTime)
    {}

    virtual void endOfSequence() override {
        doFlush();
    }

    virtual void operator <<(const Entry &ent) override {
        if (ent.time - lastFlushTime > dtBinSize) {
            onBinOverflow(ent);
        } else {
            onBinUnderflow(ent);
        }
    }

    inline double getDtBinSize() const { return dtBinSize; }
};

/// based on "dadd", "drem" events.
class DfMemUsage : public TimeHistCollector {

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
protected:

    virtual void flushBin(double binStartTime) override {
        storeFn(binStartTime, usage);
    }

    virtual void accumulateBin(const Entry &ent) override {
        TagType tt = getTagType(ent);
        switch (tt) {
            case NOTAG: return;
            case DADD: usage += getSize(ent); break;
            case DREM: usage -= getSize(ent); break;
        }
    }

public:
    DfMemUsage(std::function<void(double,ssize_t)> storeFn,
               double dtBinSize = 1e-15, double initialTime = 0)
        : TimeHistCollector(dtBinSize, initialTime)
        , tagFinder("\"tag\":")
        , sizeFinder("\"size\":")
        , storeFn(storeFn)
    {}
};

};

#endif // STATCOLLECTORS_HPP_

