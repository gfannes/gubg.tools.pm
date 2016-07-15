#ifndef HEADER_tt_Options_hpp_ALREADY_INCLUDED
#define HEADER_tt_Options_hpp_ALREADY_INCLUDED

#include <string>

namespace tt { 
    struct Options
    {
        bool print_help = false;
        std::string input_fn;
    };
} 

#endif
