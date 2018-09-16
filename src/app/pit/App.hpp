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
                std::cout << "   Aggregated    |       Node      " << std::endl;
                std::cout << "Total Todo Progr | Total Todo Progr" << std::endl;
                std::cout << "-----------------------------------" << std::endl;
                auto lambda = [&](const auto &node)
                {
                    if (node.total_duration.as_minutes() == 0)
                        std::cout << "----- ";
                    else
                        std::cout << std::setw(2) << node.total_duration << ' ';

                    if (node.total_duration.as_minutes() == 0 && node.total_todo.as_minutes() == 0)
                        std::cout << "----- ";
                    else
                        std::cout << std::setw(2) << node.total_todo << ' ';

                    {
                        const double duration = node.total_duration.as_minutes();
                        if (duration > 0)
                        {
                            const double todo = node.total_todo.as_minutes();
                            if (todo == 0.0)
                                std::cout << "DONE ";
                            else
                            {
                                const double progress = 1.0-todo/duration;
                                std::cout << std::setw(3) << std::lround(progress*100.0) << "% ";
                            }
                        }
                        else
                            std::cout << "---- ";
                    }

                    std::cout << "| ";

                    if (node.duration)
                        std::cout << std::setw(2) << *node.duration << ' ';
                    else
                        std::cout << "----- ";
                    if (node.todo)
                        std::cout << std::setw(2) << *node.todo << ' ';
                    else
                        std::cout << "----- ";
                    if (node.duration)
                    {
                        const double duration = node.duration->as_minutes();
                        MSS(duration > 0);
                        const double todo = node.todo ? node.todo->as_minutes() : duration;
                        const double progress = 1.0-todo/duration;
                        std::cout << std::setw(3) << std::lround(progress*100.0) << "% ";
                    }
                    else
                        std::cout << "---- ";
                    std::cout << std::string(node.depth()*2, ' ') << node.tag << std::endl;
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
