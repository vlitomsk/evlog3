#ifndef CONSUMER_HPP_
#define CONSUMER_HPP_

#include "entry.hpp"
#include <memory>

struct Consumer {
    virtual void operator <<(const Entry &ent) = 0;
};
typedef std::shared_ptr<Consumer> ConsumerPtr;

#endif // CONSUMER_HPP_

