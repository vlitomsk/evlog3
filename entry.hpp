#ifndef ENTRY_HPP_
#define ENTRY_HPP_

#include <string>
#include <vector>

struct Entry {
    double time;
    std::string str;

    friend void swap(Entry &a, Entry &b);
    bool operator >(const Entry &ent) const;
};

typedef std::vector<Entry> EntryBuf;

#endif // ENTRY_HPP_

