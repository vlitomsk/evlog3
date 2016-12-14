#ifndef CONSUMER_HPP_
#define CONSUMER_HPP_

#include "entry.hpp"
#include <memory>

struct Consumer {
    virtual void operator <<(const Entry &ent) = 0;
    virtual void endOfSequence() = 0;
};

using ConsumerPtr = std::shared_ptr<Consumer>;

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
        lastEntryTime = ent.getTime();
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
        if (ent.getTime() - lastFlushTime > dtBinSize) {
            onBinOverflow(ent);
        } else {
            onBinUnderflow(ent);
        }
    }

    inline double getDtBinSize() const { return dtBinSize; }
};

#endif // CONSUMER_HPP_

