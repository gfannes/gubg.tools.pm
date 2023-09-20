#include <org/Options.hpp>

#include <gubg/cli/Range.hpp>
#include <gubg/mss.hpp>

namespace org {

    bool Options::init(int argc, const char **argv)
    {
        MSS_BEGIN(bool);

        gubg::cli::Range argr{argc, argv};

        MSS(argr.pop(exe_name));

        for (std::string arg; argr.pop(arg);)
        {
            auto is = [&](const char *sh, const char *lh) {
                return arg == sh || arg == lh;
            };
            if (false) {}
            else if (is("-i", "--input"))
                MSS(argr.pop(filepath.emplace()));
            else
                MSS(false, log_.error() << "Unknown argument '" << arg << "'" << std::endl);
        }

        MSS_END();
    }

    bool Options::init(EnvVars env_vars)
    {
        MSS_BEGIN(bool);

        auto getenv = [](const std::string &name, auto &&cb) {
            if (auto cstr = std::getenv(name.c_str()); !!cstr)
                cb(cstr);
        };

        switch (env_vars)
        {
            case EnvVars::Helix:
            {
                if (!filepath)
                    getenv("helix_filepath", [&](auto value) { filepath = value; });
            }
            break;
        }

        MSS_END();
    }

    void Options::write(gubg::naft::Node &p) const
    {
        auto n = p.node("Options");
        if (filepath)
            n.attr("filepath", *filepath);
    }

} // namespace org
