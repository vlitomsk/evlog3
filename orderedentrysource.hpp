#ifndef ORDEREDENTRYSOURCE_HPP_
#define ORDEREDENTRYSOURCE_HPP_

#include <functional>
#include "restrictions.hpp"
#include "entryreader.hpp"
#include "entrywriter.hpp"

class OrderedEntrySource {
public:
    typedef std::function<bool(const Entry &)> FiltFunc;

private:
    FiltFunc isSelected, isDropped;
    std::vector<ConsumerPtr> consumers;
    Restrictions restr;
    std::string tempfileTempl = "/tmp/tmpfileXXXXXX";

    std::string getTempFileName() const;
    void clearBuffers(std::vector<EntryBuf> &bufs) const;

    void fillBuffers(
        std::vector<EntryReader> &readers,
        std::vector<EntryBuf> &bufs,
        int maxBufSize,
        std::vector<size_t> &idcsToClose) const;

    void bubble_sort(std::vector<Entry> &v) const;
    void sortBuffers(std::vector<EntryBuf> &bufs) const;
    void consumeEntry(Entry &ent) const;

    void mergePropagateBuffers(
        std::vector<EntryBuf> &bufs,
        std::function<void(Entry &ent)> propagate) const;

    void mergeLogsKway(
        std::vector<std::shared_ptr<std::istream>> &iss,
        const Restrictions &restr,
        std::function<void(Entry &ent)> propagate) const;

public:
    OrderedEntrySource();
    void resetPrefilter();
    void setRestrictions(const Restrictions &restr);
    void setPrefilter(FiltFunc isSelected, FiltFunc isDropped);
    void setConsumers(const std::vector<ConsumerPtr> &consumers);
    void setTempfileTemplate(const std::string &templ);
    bool emitMerged(const std::vector<std::string> &logPaths);
};

#endif // ORDEREDENTRYSOURCE_HPP_
