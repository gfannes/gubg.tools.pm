#ifndef HEADER_pit_Types_hpp_ALREADY_INCLUDED
#define HEADER_pit_Types_hpp_ALREADY_INCLUDED

#include <string>
#include <vector>

namespace pit { 

    struct InputFile
    {
        std::string ns;
        std::string fn;
    };
    using InputFiles = std::vector<InputFile>;

} 

#endif
