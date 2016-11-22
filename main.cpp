#include <iostream>
#include <fstream>
#include "orderedentrysource.hpp"

int main(int argc, char **argv)
{
    std::vector<std::string> logPaths = {
      "/home/v/work/build-evlog3-Desktop-Default/2_0.evlog",
      "/home/v/work/build-evlog3-Desktop-Default/1_0.evlog"
    };
    //for (int i = 1; i < argc; ++i)
    //    logPaths.push_back(argv[i]);

    std::ofstream outp("output");
    Restrictions restr;
    restr.maxBufEntries = 1000;
    restr.maxOpenedFiles = 100;

    OrderedEntrySource entSource;
    ConsumerPtr writerConsumer(new EntryWriter(outp));
    entSource.setConsumers({ writerConsumer });
    entSource.setRestrictions(restr);
    entSource.emitMerged(logPaths);

    return 0;
}

