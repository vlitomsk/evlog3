#include "entrywriter.hpp"

EntryWriter::EntryWriter(std::ostream &os)
  : os(os)
  , first(true)
{}

EntryWriter::~EntryWriter() {
    os << "\n]\n";
}

void EntryWriter::operator <<(const Entry &ent) {
    if (first) {
        os << "[\n";
        first = false;
    } else {
        os << ",\n";
    }

    os << ent.str;
}
