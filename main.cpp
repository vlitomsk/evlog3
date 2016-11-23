#include <iostream>
#include <fstream>
#include "orderedentrysource.hpp"
#include "prefilters.hpp"
#include "statcollectors.hpp"
int main(int argc, char **argv)
{
    using namespace std;
    using namespace placeholders;
    using namespace stats;
    vector<string> logPaths = {
        "/home/v/work/build-evlog3-Desktop-Default/1_0.evlog",
        "/home/v/work/build-evlog3-Desktop-Default/2_0.evlog"
    };
    //for (int i = 1; i < argc; ++i)
    //    logPaths.push_back(argv[i]);

    ofstream outp("output");
    Restrictions restr;
    restr.maxBufEntries = 1000;
    restr.maxOpenedFiles = 100;

    AnyOfPrefilter prefilter({
        TagPrefilter("dadd"),
        TagPrefilter("drem")
    });

    OrderedEntrySource entSource;
    entSource.setPrefilter(
                //bind(&AnyOfPrefilter::matches, &prefilter, _1),
                [](const Entry &) { return true; },
                [](const Entry &) { return false; }
    );

    ofstream outUsage("dfMemUsage");
    ConsumerPtr writerConsumer(new EntryWriter(outp));
    ConsumerPtr dfMemUsageConsumer(new DfMemUsage(
        [&outUsage](double time, ssize_t usage) {
            outUsage << time << ' ' << usage << endl;
        }));
    entSource.setConsumers({ writerConsumer, dfMemUsageConsumer });
    entSource.setRestrictions(restr);
    entSource.emitMerged(logPaths);

    return 0;
}

