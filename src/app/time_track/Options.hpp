#ifndef HEADER_time_track_Options_hpp_ALREADY_INCLUDED
#define HEADER_time_track_Options_hpp_ALREADY_INCLUDED

#include <time_track/enums.hpp>

#include <gubg/Logger.hpp>

#include <string>
#include <optional>

namespace time_track { 
    struct Options
    {
        std::string exe_name;

        bool print_help = false;
        bool print_totals = false;
        int verbose_level = 0;
        std::string input_fn;

        std::optional<YYYYMMDD> from;
        std::optional<YYYYMMDD> until;

        bool parse(int argc, const char **argv, gubg::Logger &);

        std::string help() const;
    };
} 

#endif
