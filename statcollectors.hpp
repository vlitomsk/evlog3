#ifndef STATCOLLECTORS_HPP_
#define STATCOLLECTORS_HPP_

#include "consumer.hpp"
#include "prefilters.hpp"
#include <unordered_map>

namespace stats {

/// based on "dadd", "drem" events.
class DfMemUsage : public TimeHistCollector {
    ssize_t usage;
    std::function<void(double,decltype(usage))> storeFn;
protected:
    virtual void flushBin(double binStartTime) override {
        storeFn(binStartTime, usage);
    }

    virtual void accumulateBin(const Entry &ent) override {
        auto tt = ent.getTagType();
        switch (tt) {
            case Entry::DADD: usage += ent.getDfSize(); break;
            case Entry::DREM: usage -= ent.getDfSize(); break;
            default: return;
        }
    }
public:
    DfMemUsage(std::function<void(double,ssize_t)> storeFn,
               double dtBinSize = 1e-15, double initialTime = 0)
        : TimeHistCollector(dtBinSize, initialTime)
        , storeFn(storeFn)
    {}
};

class SendDistanceDistrib : public TimeHistCollector {
    std::unordered_map<int, int> distr;
    std::function<void(double,decltype(distr))> storeFn;
protected:

public:
    SendDistanceDistrib(std::function<void(double,decltype(distr))> storeFn,
                        double dtBinSize = 1e-15, double initialTime = 0)
        : TimeHistCollector(dtBinSize, initialTime)
        , storeFn(storeFn)
    {}
};

};

#endif // STATCOLLECTORS_HPP_

