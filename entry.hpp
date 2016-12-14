#ifndef ENTRY_HPP_
#define ENTRY_HPP_

#include <string>
#include <vector>
#include "suspension.hpp"
#include <iostream>
#include "cachedstringfinder.hpp"
#include <unordered_map>
#include <boost/algorithm/string.hpp>

class Entry {
public:
    typedef enum {
        NOTAG, // has no tag
        DCREATE, DADD, DREM, DSEND, DRECV, DMEMADD, DMEMREM, // DF event tags
        CSUBMIT, CSEND, CRECV, CREADY, CSTART, CSTOP, CFINISH, // CF event tags
        SEND, RECV, // Any comm. msg tags
        LDUPD // DLB event tags
    } TagType;

    class EntryParser {
        mutable std::unordered_map<std::string, CachedStringFinder> finders;
        const std::unordered_map<std::string, Entry::TagType> tagMap {
            {"DCREATE", DCREATE},
            {"DADD", DADD},
            {"DREM", DREM},
            {"DSEND", DSEND},
            {"DRECV", DRECV},
            {"DMEMADD", DMEMADD},
            {"DMEMREM", DMEMREM},
            {"CSUBMIT", CSUBMIT},
            {"CSEND", CSEND},
            {"CRECV", CRECV},
            {"CREADY", CREADY},
            {"CSTART", CSTART},
            {"CSTOP", CSTOP},
            {"CFINISH", CFINISH},
            {"SEND", SEND},
            {"RECV", RECV},
            {"LDUPD", LDUPD}
        }; // TODO convert to static

        inline CachedStringFinder & getFinder(const std::string &field) const {
            if (!finders.count(field)) {
                finders.emplace(std::make_pair(field, CachedStringFinder(field)));
            }
            return finders.at(field);
        }

    public:
        EntryParser() = default;

        void parse(const std::string &str, const std::string &field, bool &success, double &result) const {
            auto &finder = getFinder(field);
            size_t pos = finder.findIn(str);
            result  = (success = pos != std::string::npos)
                    ? strtod(str.data() + pos + finder.getSubstrLen() + 1, NULL)
                    : 0;
        }

        void parse(const std::string &str, const std::string &field, bool &success, int &result) const {
            auto &finder = getFinder(field);
            size_t pos = finder.findIn(str);
            result  = (success = pos != std::string::npos)
                    ? strtol(str.data() + pos + finder.getSubstrLen() + 1, NULL, 10)
                    : 0;
        }

        void parse(const std::string &str, const std::string &field, bool &success, std::string &result) const {
            auto &finder = getFinder(field);
            size_t pos = finder.findIn(str);
            auto start = pos + finder.getSubstrLen() + 2; // :"
            result  = (success = pos != std::string::npos)
                    ? str.substr(start, str.find('"', start) - start)
                    : "";
        }

        void parse(const std::string &str, const std::string &field, bool &success, TagType &result) const {
            std::string strType;
            parse(str, field, success, strType);
            boost::to_upper(strType);
            result  = success && tagMap.count(strType)
                    ? tagMap.at(strType)
                    : NOTAG;
        }
    };

private:

    template <typename T>
    class suspension{
       mutable T value;
       mutable bool initialized;
       mutable bool succ;
    public:
       suspension():
          initialized(false){}

       inline T const & get(std::shared_ptr<EntryParser> eparser, const std::string &s, const std::string &field) const{
          if (!initialized){
             eparser->parse(s, field, succ, value);
             initialized=true;
          }
          return value;
       }
    };

    std::string str;
    std::shared_ptr<EntryParser> ep;
    bool _succ;

    suspension<double> time;
    suspension<TagType> tag;
    suspension<int> node;
    suspension<int> dfId;
    suspension<int> dfSize;

public:

    Entry(const std::string &str = "", std::shared_ptr<EntryParser> entryParser = nullptr)
        : str(str), ep(entryParser)
    {}

    inline void setEntryParser(std::shared_ptr<EntryParser> entryParser) { this->ep = entryParser; }

    inline void setStr(const std::string &s) {
        auto len = str.length();
        if (len == 0)
            str = s;
        else
            throw std::runtime_error("Not implemented yet. Add resetting of parsed values!");
    }

    inline const std::string & getStr() const { return str; }
    inline double getTime() const { return time.get(ep, str, "\"time\""); }
    inline TagType getTagType() const { return tag.get(ep, str, "\"tag\""); }
    inline int getNodeRank() const { return node.get(ep, str, "\"node\""); }
    inline int getDfId() const { return dfId.get(ep, str, "\"id\""); }
    inline size_t getDfSize() const { return dfSize.get(ep, str, "\"size\""); }

    friend void swap(Entry &a, Entry &b);
    bool operator >(const Entry &ent) const;
};

typedef std::vector<Entry> EntryBuf;

#endif // ENTRY_HPP_

