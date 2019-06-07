#ifndef HEADER_pit_Options_hpp_ALREADY_INCLUDED
#define HEADER_pit_Options_hpp_ALREADY_INCLUDED

#include <pit/Types.hpp>
#include <gubg/mss.hpp>
#include <gubg/Strange.hpp>
#include <string>
#include <ostream>
#include <set>
#include <vector>

namespace pit { 

    class Options
    {
    public:
        std::string exe_fn;
        bool verbose = false;
        bool help = false;

        InputFiles input_files;
        Mode mode = Mode::Report;
        bool show_xlinks = false;
        std::optional<std::string> uri;
        std::optional<unsigned int> depth;

        bool parse(int argc, const char **argv)
        {
            MSS_BEGIN(bool);
            MSS(argc > 0);
            exe_fn = argv[0];
            std::set<std::string> switches = {"-v", "-h"};
            std::set<std::string> options = {"-f", "-u", "-d", "-m"};
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
                    if (arg == "-f")
                    {
                        InputFile input_file;
                        gubg::Strange strange{value};
                        strange.pop_until(input_file.ns, ':');
                        strange.pop_all(input_file.fn);
                        input_files.push_back(input_file);
                    }
                    if (arg == "-u") { uri = value; }
                    if (arg == "-d") { depth = std::stoi(value); }
                    if (arg == "-m")
                    {
                        gubg::Strange strange(value);
                        if (strange.pop_if("plan")) { mode = Mode::Plan; }
                        else if (strange.pop_if("report"))
                        {
                            mode = Mode::Report;
                            show_xlinks = strange.pop_if(".full");
                        }
                        MSS(strange.empty(), std::cout << "Error: unknown mode \"" << value << "\"" << std::endl);
                    }
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
            for (const auto &input_file: input_files)
                os << "  input file: " << input_file.ns << " : " << input_file.fn << std::endl;
            os << "  verbose: " << verbose << std::endl;
            os << "  help: " << help << std::endl;
            os << "  mode: " << mode << std::endl;
            os << "  show_xlinks: " << show_xlinks << std::endl;
            if (uri)
                os << "  uri: " << *uri << std::endl;
            if (depth)
                os << "  depth: " << *depth << std::endl;
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
