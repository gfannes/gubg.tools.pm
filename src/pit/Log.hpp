#ifndef HEADER_pit_Log_hpp_ALREADY_INCLUDED
#define HEADER_pit_Log_hpp_ALREADY_INCLUDED

#include <ostream>

namespace pit { 

    class Options;

    void configure_log(const Options &options);

    enum LogLevel
    {
        Error,
        Verbose,
        Debug,
    };

    std::ostream &log(LogLevel);

} 

#endif
