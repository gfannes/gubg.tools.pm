#ifndef HEADER_time_track_Options_hpp_ALREADY_INCLUDED
#define HEADER_time_track_Options_hpp_ALREADY_INCLUDED

#include <string>

namespace time_track { 
    struct Options
    {
        bool print_help = false;
        std::string input_fn;
        int year = -1;
        int month = -1;
        int day = 1;
    };
} 

#endif
