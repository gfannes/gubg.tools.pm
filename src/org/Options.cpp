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
            else if (is("-m", "--mode"))
            {
                std::string mode_str;
                MSS(argr.pop(mode_str));
                if (mode_str == "normal")
                    mode = Mode::Normal;
                else if (mode_str == "lsp")
                    mode = Mode::LSP;
                else
                    MSS(false, log_.error() << "Unknown mode '" << mode_str << "'" << std::endl);
            }
            else if (is("-l", "--log"))
                MSS(argr.pop(log_filepath));
            else if (is("-i", "--input"))
                MSS(argr.pop(filepath));
            else if (is("-r", "--range"))
            {
                MSS(argr.pop(arg));
                gubg::Strange strange{arg};
                std::size_t begin, size;
                MSS(strange.pop_decimal(begin));
                MSS(strange.pop_if(':'));
                MSS(strange.pop_decimal(size));

                ranges.emplace_back(begin, size);
            }
            else if (is("-p", "--primary"))
                MSS(argr.pop(primary));
            else if (is("-s", "--state"))
                MSS(argr.pop(state));
            else if (is("-t", "--tag"))
                MSS(argr.pop(tag.emplace()));
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
                            std::size_t begin, size;
                            MSS(strange.pop_decimal(begin));
                            MSS(strange.pop_if(':'));
                            MSS(strange.pop_decimal(size));
                            ranges.emplace_back(begin, size);
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
        oss << "    -h --help    Print this help" << std::endl;
        oss << "    -m --mode    Mode: normal|lsp [default is normal]" << std::endl;
        oss << "    -l --log     Filepath used for logging" << std::endl;
        oss << "    -i --input   Input filepath" << std::endl;
        oss << "    -r --range   Range in format 'index:size'" << std::endl;
        oss << "    -p --primary Primary range" << std::endl;
        oss << "    -s --state   New state for primary range" << std::endl;
        oss << "    -t --tag     New tag for primary range" << std::endl;
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
            nn.attr("start", range.start()).attr("size", range.size());
        }
    }

} // namespace org
