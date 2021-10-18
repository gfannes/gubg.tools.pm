#ifndef HEADER_time_track_Options_hpp_ALREADY_INCLUDED
#define HEADER_time_track_Options_hpp_ALREADY_INCLUDED

#include <string>
#include <optional>

namespace time_track { 
    struct Options
    {
        bool print_help = false;
        bool print_totals = false;
        std::string input_fn;

        struct YYYYMMDD
        {
            int year = -1;
            int month = -1;
            int day = 1;
        };
        std::optional<YYYYMMDD> from;
        std::optional<YYYYMMDD> until;
    };
} 

#endif
