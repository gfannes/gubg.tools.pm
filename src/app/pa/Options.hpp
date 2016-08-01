#ifndef HEADER_pa_Options_hpp_ALREADY_INCLUDED
#define HEADER_pa_Options_hpp_ALREADY_INCLUDED

#include "pa/Value2Days.hpp"
#include "gubg/file/File.hpp"
#include <string>
#include <vector>

namespace pa { 

    struct Options
    {
        gubg::file::File input;
        gubg::file::File workers;
        gubg::file::File output;
        std::string value;
        Value2Days value2days;
        std::string fraction;
        std::string fraction_default;
		std::string category;
		std::vector<std::string> lines;
    };

} 

#endif
