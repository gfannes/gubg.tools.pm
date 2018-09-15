#ifndef HEADER_pit_App_hpp_ALREADY_INCLUDED
#define HEADER_pit_App_hpp_ALREADY_INCLUDED

#include "pit/Options.hpp"
#include "pit/Config.hpp"
#include "pit/Model.hpp"
#include "gubg/mss.hpp"
#include <cstdlib>
#include <set>
#include <iomanip>
#include <cmath>

namespace pit { 
    class App
    {
    public:
        bool process(int argc, const char **argv)
        {
            MSS_BEGIN(bool);

            MSS(options_.parse(argc, argv), std::cout << "Error: could not parse cli args" << std::endl);

            {
                auto config_path = config_path_();
                config_.install(config_path);
                MSS(config_.load(config_path), std::cout << "Error: could not load config" << std::endl);
            }

            if (options_.verbose)
                std::cout << options_;
            if (options_.help)
                options_.stream_help(std::cout);

            const auto &command = options_.command;

            std::set<std::string> command_needs_model = {"tree"};
            if (command_needs_model.count(command))
                MSS(model_.load(options_.input_fn));

            if (false) {}
            else if (command == "tree")
            {
                int level = -1;
                auto lambda = [&](const Model::Node &node, bool enter)
                {
                    if (enter)
                    {
                        level++;
                        std::cout << std::setw(3) << node.duration << ' ';
                        std::cout << std::setw(3) << std::lround(node.progress*100.0) << "%";
                        std::cout << std::string(level*2, ' ') << node.tag << std::endl;
                    }
                    else
                    {
                        level--;
                    }
                    return true;
                };
                MSS(model_.dfs(lambda));
            }
            else MSS(false, std::cout << "Error: unknown command \"" << command << "\"" << std::endl);
            MSS_END();
        }

    private:
        std::filesystem::path config_path_() const
        {
            std::filesystem::path res("pit_config.naft");
            auto home = std::getenv("HOME");
            if (home)
            {
                res = home;
                res /= ".config";
                res /= "pit";
                res /= "config.naft";
            }
            return res;
        }

        Options options_;
        Config config_;
        Model model_;
    };
} 

#endif
