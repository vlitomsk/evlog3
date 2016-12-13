#ifndef ENTRYWRITER_HPP_
#define ENTRYWRITER_HPP_

#include <iostream>
#include "entry.hpp"
#include "consumer.hpp"

class EntryWriter : public Consumer {
    std::ostream &os;
    bool first;
public:
    EntryWriter(std::ostream &os);
    ~EntryWriter();
    virtual void operator <<(const Entry &ent) override;
    virtual void endOfSequence() override;
};

#endif // ENTRYWRITER_HPP_
