#ifndef HEADER_org_Options_hpp_ALREAD_INCLUDED
#define HEADER_org_Options_hpp_ALREAD_INCLUDED

#include <gubg/naft/Node.hpp>
#include <gubg/naft/stream.hpp>
#include <gubg/Logger.hpp>

#include <cstdlib>
#include <optional>
#include <string>

namespace org {

    enum class EnvVars
    {
        Helix,
    };

    class Options
    {
    public:
        std::string exe_name;

        std::optional<std::string> filepath;

        bool init(int argc, const char **argv);
        bool init(EnvVars);

        void write(gubg::naft::Node &) const;

    private:
        gubg::Logger log_;
    };

} // namespace org

#endif
