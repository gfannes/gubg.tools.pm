#ifndef HEADER_org_Options_hpp_ALREAD_INCLUDED
#define HEADER_org_Options_hpp_ALREAD_INCLUDED

#include <org/types.hpp>

#include <gubg/Logger.hpp>
#include <gubg/naft/Node.hpp>
#include <gubg/naft/stream.hpp>

#include <cstdlib>
#include <string>
#include <vector>

namespace org {

    enum class EnvVars
    {
        Helix,
    };

    struct Range
    {
        Ix begin = 0;
        Size size = 0;
    };

    class Options
    {
    public:
        std::string exe_name;

        bool print_help = false;
        std::string filepath;
        std::vector<Range> ranges;
        Ix primary = 0;

        bool init(int argc, const char **argv);
        bool init(EnvVars);

        std::string help() const;

        void write(gubg::naft::Node &) const;

    private:
        gubg::Logger log_;
    };

} // namespace org

#endif
