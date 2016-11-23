#ifndef PREFILTERS_HPP_
#define PREFILTERS_HPP_

#include "cachedstringfinder.hpp"
#include "entry.hpp"

class SubstrPrefilter {
    const CachedStringFinder finder;
public:
    SubstrPrefilter(const std::string &substr);
    inline bool matches(const Entry &ent) const {
        return finder.findIn(ent.str) != std::string::npos;
    }

    inline bool nmatches(const Entry &ent) const { return !matches(ent); }
};

struct FieldPrefilter : public SubstrPrefilter {
    FieldPrefilter(const std::string &field, const std::string &value);
};

struct TagPrefilter : public FieldPrefilter {
    TagPrefilter(const std::string &tag);
};

#include <vector>
#include <algorithm>
struct AnyOfPrefilter {
    const std::vector<SubstrPrefilter> filts;
public:
    AnyOfPrefilter(const std::vector<SubstrPrefilter> &filts)
        : filts(filts)
    {}

    inline bool matches(const Entry &ent) const {
        return std::any_of(filts.begin(), filts.end(),
                [&ent](const auto &filt) { return filt.matches(ent); });
    }

    inline bool nmatches(const Entry &ent) const { return !matches(ent); }
};

#endif // PREFILTERS_HPP_

