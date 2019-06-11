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
        bool plan = false;
        bool show_xlinks = false;
        std::optional<std::string> resources_fn;
        std::optional<std::string> uri;
        std::optional<unsigned int> depth;
        unsigned int nr_work_days = 20;
        std::optional<std::string> output_fn;

        bool parse(int argc, const char **argv)
        {
            MSS_BEGIN(bool);
            MSS(argc > 0);
            exe_fn = argv[0];
            std::set<std::string> switches = {"-v", "-h", "-p"};
            std::set<std::string> options = {"-f", "-u", "-d", "-r", "-n", "-o"};
            for (auto i = 1; i < argc;)
            {
                std::string arg = argv[i++];
                if (false) {}
                else if (switches.count(arg))
                {
                    if (arg == "-v") { verbose = true; }
                    if (arg == "-h") { help = true; }
                    if (arg == "-p") { plan = true; }
                    if (arg == "-x") { show_xlinks = true; }
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
                    if (arg == "-n") { nr_work_days = std::stol(value); }
                    if (arg == "-r") { resources_fn = value; }
                    if (arg == "-o") { output_fn = value; }
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
            os << "  plan: " << plan << std::endl;
            os << "  show_xlinks: " << show_xlinks << std::endl;
            os << "  nr_work_days: " << nr_work_days << std::endl;
            if (resources_fn)
                os << "  resources_fn: " << *resources_fn << std::endl;
            if (output_fn)
                os << "  output_fn: " << *output_fn << std::endl;
            if (uri)
                os << "  uri: " << *uri << std::endl;
            if (depth)
                os << "  depth: " << *depth << std::endl;
        }
        void stream_help(std::ostream &os) const
        {
            os << exe_fn << " <options>* <command>? <arguments>*\n";
            os << "  -f [namespace:]filename Input filename\n";
            os << "  -v                      Verbose\n";
            os << "  -h                      Print this help\n";
            os << "  -u uri                  URI\n";
            os << "  -d depth                Depth\n";
            os << "  -p                      Enable planning\n";
            os << "  -r filename             Resource filename\n";
            os << "  -n nr                   Nr work days to plan\n";
            os << "  -o filename             Output to file\n";
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
