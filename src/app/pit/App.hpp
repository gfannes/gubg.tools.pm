#ifndef HEADER_pit_App_hpp_ALREADY_INCLUDED
#define HEADER_pit_App_hpp_ALREADY_INCLUDED

#include "pit/Options.hpp"
#include "gubg/mss.hpp"

namespace pit { 
    class App
    {
    public:
        bool parse(int argc, const char **argv)
        {
            MSS_BEGIN(bool);
            MSS(options_.parse(argc, argv));
            MSS_END();
        }

        bool process()
        {
            MSS_BEGIN(bool);
            MSS_END();
        }

    private:
        Options options_;
    };
} 

#endif
