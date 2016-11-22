#include "entry.hpp"

void swap(Entry &a, Entry &b) {
    using std::swap;
    swap(a.time, b.time);
    swap(a.str, b.str);
}

bool Entry::operator >(const Entry &ent) const {
    return time > ent.time;
}

