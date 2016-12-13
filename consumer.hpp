#ifndef CONSUMER_HPP_
#define CONSUMER_HPP_

#include "entry.hpp"
#include <memory>

struct Consumer {
    virtual void operator <<(const Entry &ent) = 0;
    virtual void endOfSequence() = 0;
};

using ConsumerPtr = std::shared_ptr<Consumer>;

#endif // CONSUMER_HPP_

