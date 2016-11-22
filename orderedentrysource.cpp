#include "orderedentrysource.hpp"
#include <fstream>
#include <unistd.h> // for unlink()
#include <queue>
#include <set>

void OrderedEntrySource::
clearBuffers(std::vector<EntryBuf> &bufs) const {
    for (auto &eb : bufs)
        eb.clear();
}


void OrderedEntrySource::
fillBuffers(
    std::vector<EntryReader> &readers,
    std::vector<EntryBuf> &bufs,
    int maxBufSize,
    std::vector<size_t> &idcsToClose) const
{
    const size_t sz = readers.size();
    for (size_t i = 0; i < sz; ++i) {
        size_t j;
        for (j = 0; j < maxBufSize && !readers[i].isEof(); ++j) {
            bufs[i].emplace_back();
            readers[i] >> bufs[i][j];
        }
        if (readers[i].isEof())
            idcsToClose.push_back(i);
        bufs[i].resize(j);
    }
}


void OrderedEntrySource::
bubble_sort(std::vector<Entry> &v) const {
    if (v.empty()) return;

    const size_t sz = v.size();
    for (size_t i = 1; i < sz; ++i) {
        if (v[i-1] > v[i]) {
            int j = i;
            while (j > 0 && v[j-1] > v[j]) {
                std::swap(v[j], v[j-1]);
                --j;
            }
        }
    }
}


void OrderedEntrySource::
sortBuffers(std::vector<EntryBuf> &bufs) const
{
    for (auto &buf : bufs) {
        bubble_sort(buf);
    }
}


void OrderedEntrySource::
consumeEntry(Entry &ent) const {
    for (auto cons : consumers) {
        *cons << ent;
    }
}


void OrderedEntrySource::
mergePropagateBuffers(
    std::vector<EntryBuf> &bufs,
    std::function<void(Entry &ent)> propagate) const
{

    typedef std::pair<size_t, Entry*> MergeSetType;
    struct MergeSetComp {
        bool operator () (const MergeSetType &l, const MergeSetType &r) const {
            // multiset's .begin() would point to MIN ; .rbegin() -- to MAX
            return l.second->time < r.second->time;
        }
    };

    std::multiset<MergeSetType, MergeSetComp> mergeSet;
    std::vector<size_t> fronts(bufs.size(), 0);

    for (size_t i = 0; i < bufs.size(); ++i) {
        if (!bufs[i].empty()) {
            mergeSet.insert(std::make_pair(i, &bufs[i][0]));
        }
    }

    while (!mergeSet.empty()) {
        auto minPair = *mergeSet.begin();
        mergeSet.erase(mergeSet.begin());
        if (++fronts[minPair.first] < bufs[minPair.first].size()) {
            mergeSet.emplace(std::make_pair(
                minPair.first,
                &bufs[minPair.first][fronts[minPair.first]]));
        }
        propagate(*(minPair.second));
    }
}


void OrderedEntrySource::
mergeLogsKway(
    std::vector<std::shared_ptr<std::istream>> &iss,
    const Restrictions &restr,
    std::function<void(Entry &ent)> propagate) const
{
    std::vector<EntryReader> readers;
    std::vector<EntryBuf> entryBufs;
    const int bufSize = restr.maxBufEntries / iss.size();
    for (auto &is : iss) {
        readers.emplace_back(*is);
        entryBufs.emplace_back();
        entryBufs.back().reserve(bufSize);
    }

    int readersLeft = readers.size();
    while (readersLeft > 0) {
        std::vector<size_t> idcsToClose;
        clearBuffers(entryBufs);
        fillBuffers(readers, entryBufs, bufSize, idcsToClose);
        sortBuffers(entryBufs);
        mergePropagateBuffers(entryBufs, propagate);
        readersLeft -= idcsToClose.size();
    }
}


std::shared_ptr<std::ostream> OrderedEntrySource::
getTempOstream(std::string &filePath) {
    filePath = "/tmp/tmpfileXXXXXX";
    mkstemp(&filePath[0]);
    return std::shared_ptr<std::ostream>(new std::ofstream(filePath));
}

OrderedEntrySource::
OrderedEntrySource()
{
    resetPrefilter();
    restr.maxBufEntries = 1000;
    restr.maxOpenedFiles = 1000;
}

void OrderedEntrySource::
resetPrefilter() {
    setPrefilter(
        [](const Entry &) { return true; },
        [](const Entry &) { return false; } );
}

void OrderedEntrySource::
setRestrictions(const Restrictions &restr) {
    this->restr = restr;
}

void OrderedEntrySource::
setPrefilter(FiltFunc isSelected, FiltFunc isDropped) {
    this->isSelected = isSelected;
    this->isDropped = isDropped;
}

void OrderedEntrySource::
setConsumers(const std::vector<ConsumerPtr> &consumers) {
    this->consumers = consumers;
}

void OrderedEntrySource::
emitMerged(const std::vector<std::string> &logPaths) {
    using namespace std;
    using namespace std::placeholders;
    queue<pair<bool, string>> pathsToMerge;
    for (auto p : logPaths) {
        pathsToMerge.push({false, p});
    }

    while (!pathsToMerge.empty()) {
        vector<shared_ptr<istream>> iss;

        const auto i_to = min(restr.maxOpenedFiles, (int)pathsToMerge.size());
        for (int i = 0; i < i_to; ++i) {
            const auto &front = pathsToMerge.front();
            iss.push_back(shared_ptr<istream>(new ifstream(front.second)));
            if (front.first) {
                unlink(front.second.c_str());
            }
            pathsToMerge.pop();
        }

        if (pathsToMerge.empty()) {
            mergeLogsKway(iss, restr,
                          bind(&OrderedEntrySource::consumeEntry, this, _1));
        } else {
            string filePath;
            auto tempOsPtr = getTempOstream(filePath);
            EntryWriter tmpWriter(*tempOsPtr);
            mergeLogsKway(iss, restr,
                [&](Entry &ent) { tmpWriter << ent; });
            pathsToMerge.push({ true, move(filePath) });
        }

    }
}
