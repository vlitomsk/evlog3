#include "prefilters.hpp"

SubstrPrefilter::SubstrPrefilter(const std::string &substr)
    : finder(substr)
{}

FieldPrefilter::FieldPrefilter(const std::string &field, const std::string &value)
    : SubstrPrefilter("\"" + field + "\":\"" + value + "\"")
{}

TagPrefilter::TagPrefilter(const std::string &tag)
  : FieldPrefilter("tag", tag)
{}


