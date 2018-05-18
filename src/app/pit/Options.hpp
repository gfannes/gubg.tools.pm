#ifndef HEADER_pit_Options_hpp_ALREADY_INCLUDED
#define HEADER_pit_Options_hpp_ALREADY_INCLUDED

#include "gubg/mss.hpp"

namespace pit { 
    class Options
    {
    public:
        bool parse(int argc, const char **argv)
        {
            MSS_BEGIN(bool);
            MSS_END();
        }
    private:
    };
} 

#endif
