#include <org/Options.hpp>

#include <gubg/Strange.hpp>
#include <gubg/cli/Range.hpp>
#include <gubg/mss.hpp>
#include <gubg/string/concat.hpp>

#include <sstream>

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
            else if (is("-h", "--help"))
                print_help = true;
            else if (is("-i", "--input"))
                MSS(argr.pop(filepath));
            else if (is("-r", "--range"))
            {
                MSS(argr.pop(arg));
                gubg::Strange strange{arg};
                auto &range = ranges.emplace_back();
                MSS(strange.pop_decimal(range.begin));
                MSS(strange.pop_if(':'));
                MSS(strange.pop_decimal(range.size));
            }
            else if (is("-p", "--primary"))
                MSS(argr.pop(primary));
            else
                MSS(false, log_.error() << "Unknown argument '" << arg << "'" << std::endl);
        }

        MSS_END();
    }

    bool Options::init(EnvVars env_vars)
    {
        MSS_BEGIN(bool);

        auto getenv = [](std::string &dst, const std::string &name) {
            MSS_BEGIN(bool);
            auto cstr = std::getenv(name.c_str());
            MSS_Q(!!cstr);
            dst = cstr;
            MSS_END();
        };

        switch (env_vars)
        {
            case EnvVars::Helix:
            {
                if (filepath.empty())
                    getenv(filepath, "helix_filepath");
                if (ranges.empty())
                {
                    std::string str;
                    if (getenv(str, "helix_range_count"))
                    {
                        const Count range_count = std::stoul(str);

                        for (auto i = 0; i < range_count; ++i)
                        {
                            MSS(getenv(str, gubg::string::concat("helix_range_", i)));
                            gubg::Strange strange{str};
                            auto &range = ranges.emplace_back();
                            MSS(strange.pop_decimal(range.begin));
                            MSS(strange.pop_if(':'));
                            MSS(strange.pop_decimal(range.size));
                        }
                    }
                }
            }
            break;
        }

        MSS_END();
    }

    std::string Options::help() const
    {
        std::ostringstream oss;
        oss << "Help for " << exe_name << std::endl;
        oss << "Created by Geert Fannes" << std::endl;
        return oss.str();
    }

    void Options::write(gubg::naft::Node &p) const
    {
        auto n = p.node("Options");
        n.attr("filepath", filepath);
        for (const auto &range : ranges)
        {
            auto nn = n.node("Range");
            nn.attr("begin", range.begin).attr("size", range.size);
        }
    }

} // namespace org
