#ifndef HEADER_pit_Options_hpp_ALREADY_INCLUDED
#define HEADER_pit_Options_hpp_ALREADY_INCLUDED

#include "gubg/mss.hpp"
#include <string>
#include <ostream>
#include <set>
#include <vector>

namespace pit { 
    class Options
    {
    public:
        std::string exe_fn;
        std::string input_fn;
        bool verbose = false;
        bool help = false;
        std::string uri;
        int depth = -1;

        bool parse(int argc, const char **argv)
        {
            MSS_BEGIN(bool);
            MSS(argc > 0);
            exe_fn = argv[0];
            std::set<std::string> switches = {"-v", "-h"};
            std::set<std::string> options = {"-f", "-u", "-d"};
            for (auto i = 1; i < argc;)
            {
                std::string arg = argv[i++];
                if (false) {}
                else if (switches.count(arg))
                {
                    if (arg == "-v") { verbose = true; }
                    if (arg == "-h") { help = true; }
                }
                else if (options.count(arg))
                {
                    MSS(i < argc, std::cout << "Error: argument " << arg << " requires an value" << std::endl);
                    std::string value = argv[i++];
                    if (arg == "-f") { input_fn = value; }
                    if (arg == "-u") { uri = value; }
                    if (arg == "-d") { depth = std::stoi(value); }
                }
                else
                {
                    MSS(false, std::cout << "Error: unknown argument " << arg << std::endl);
                }
            }
            MSS_END();
        }

        void stream(std::ostream &os) const
        {
            os << "Options:" << std::endl;
            os << "  input filename: " << input_fn << std::endl;
            os << "  verbose: " << verbose << std::endl;
            os << "  help: " << help << std::endl;
            os << "  uri: " << uri << std::endl;
            os << "  depth: " << depth << std::endl;
        }
        void stream_help(std::ostream &os) const
        {
            os << exe_fn << " <options>* <command>? <arguments>*\n";
            os << "  -f <name>   Input filename\n";
            os << "  -v          Verbose\n";
            os << "  -h          Print this help\n";
            os << "  -u          URI\n";
            os << "  -d          Depth\n";
        }

    private:
    };

    inline std::ostream &operator<<(std::ostream &os, const Options &options)
    {
        options.stream(os);
        return os;
    }
} 

#endif
