#include <pit/Log.hpp>
#include <pit/Options.hpp>
#include <fstream>

namespace pit { 

    std::ostream *error = nullptr;
    std::ostream *verbose = nullptr;
    std::ostream *debug = nullptr;

    void configure_log(const Options &options)
    {
        error = &std::cout;
        if (options.verbose)
            verbose = &std::cout;
    }

    std::ostream &log(LogLevel ll)
    {
        std::ostream *os = nullptr;
        switch (ll)
        {
            case Error: os = error; break;
            case Verbose: os = verbose; break;
            case Debug: os = debug; break;
        }

        if (os)
            return *os;

        static std::ofstream devnull;
        return devnull;
    }

} 
