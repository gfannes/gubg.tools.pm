#ifndef HEADER_pit_App_hpp_ALREADY_INCLUDED
#define HEADER_pit_App_hpp_ALREADY_INCLUDED

#include "pit/Options.hpp"
#include "pit/Model.hpp"
#include "gubg/mss.hpp"
#include <cstdlib>
#include <set>

namespace pit { 
    class App
    {
    public:
        bool parse(int argc, const char **argv)
        {
            MSS_BEGIN(bool);
            MSS(options_.parse(argc, argv));
            if (options_.input_fn.empty())
            {
                auto pit_default = std::getenv("pit_default");
                if (pit_default)
                    options_.input_fn = pit_default;
            }
            MSS_END();
        }

        bool process()
        {
            MSS_BEGIN(bool);
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
        Options options_;
        Model model_;
    };
} 

#endif
