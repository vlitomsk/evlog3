#include "entry.hpp"

void swap(Entry &a, Entry &b) {
    using std::swap;

    swap(a.str, b.str);
    swap(a.ep, b.ep);

    swap(a.time, b.time);
    swap(a.tag, b.tag);
    swap(a.node, b.node);
    swap(a.dfId, b.dfId);
    swap(a.dfSize, b.dfSize);
}

bool Entry::operator >(const Entry &ent) const {
    return getTime() > ent.getTime();
}

