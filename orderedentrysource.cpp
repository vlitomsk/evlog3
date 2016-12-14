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
            Entry entry;
            readers[i] >> entry;
            //entry.setEntryParser(entryParser);
            if (!isDropped(entry) && isSelected(entry)) {
                bufs[i].push_back(entry);
                bufs[i].back().setEntryParser(entryParser);
            }
        }
        if (readers[i].isEof())
            idcsToClose.push_back(i);
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
            return l.second->getTime() < r.second->getTime();
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


OrderedEntrySource::
OrderedEntrySource()
{
    resetPrefilter();
    restr.maxBufEntries = 1000;
    restr.maxOpenedFiles = 1000;
    entryParser = std::make_shared<Entry::EntryParser>();
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


class AutoRmFile {
    std::string filePath;
    bool autoRm;
public:
    AutoRmFile(const std::string &filePath, bool autoRm)
        : filePath(filePath)
        , autoRm(autoRm)
    {}

    std::shared_ptr<std::ostream> openWrite() {
        auto ptr = std::shared_ptr<std::ostream>(new std::ofstream(filePath));
        if (!*ptr)
            throw std::invalid_argument(
                    std::string("File can't be opened for writing: ") + filePath);
        return ptr;
    }

    std::shared_ptr<std::istream> openRead() {
        auto ptr = std::shared_ptr<std::istream>(new std::ifstream(filePath));
        if (!*ptr)
            throw std::invalid_argument(
                    std::string("File can't be opened for reading: ") + filePath);
        return ptr;
    }

    const std::string & getFilePath() const { return filePath; }
    bool isAutoRm() const { return autoRm; }

    ~AutoRmFile() {
        if (autoRm)
            unlink(filePath.c_str());
    }
};


std::string OrderedEntrySource::
getTempFileName() const {
    std::string name = tempfileTempl;
    mkstemp(&name[0]);
    return name;
}


void OrderedEntrySource::
setTempfileTemplate(const std::string &templ) {
    tempfileTempl = templ;
}


bool OrderedEntrySource::
emitMerged(const std::vector<std::string> &logPaths) {
    using namespace std;
    using namespace std::placeholders;
    queue<AutoRmFile> pathsToMerge;
    for (auto p : logPaths) {
        pathsToMerge.emplace(p, false);
    }

    while (!pathsToMerge.empty()) {
        vector<shared_ptr<istream>> iss;

        for (int i = min(restr.maxOpenedFiles, (int)pathsToMerge.size()); i > 0; --i) {
            iss.push_back(pathsToMerge.front().openRead());
            pathsToMerge.pop();
        }

        if (pathsToMerge.empty()) {
            // It's the last merge, so feed subscribers with algo result
            mergeLogsKway(iss, restr,
                          bind(&OrderedEntrySource::consumeEntry, this, _1));
        } else {
            // It's not the last merge, so remember algo result in temp files
            pathsToMerge.emplace(getTempFileName(), true);
            EntryWriter tmpWriter(*pathsToMerge.back().openWrite());
            mergeLogsKway(iss, restr, [&](Entry &ent) { tmpWriter << ent; });
        }
    }
}
