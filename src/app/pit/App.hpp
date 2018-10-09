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

            MSS(model_.load(options_.input_fn));

            {
                auto start_node = model_.root();
                if (!options_.uri.empty())
                {
                    const auto &uri = options_.uri;
                    std::string error;
                    auto ptr = model_.resolve(uri, *start_node, &error);
                    MSS(!!ptr, std::cout << "Error: could not find node " << uri << ": " << error << std::endl);
                    start_node = ptr;
                }

                std::cout << " Aggregated               | Node                     | Tree" << std::endl;
                std::cout << " Total     Todo      Prog | Total     Todo      Prog |" << std::endl;
                std::cout << "--------------------------|--------------------------|---------------------------------" << std::endl;

                unsigned int depth = 0;
                std::list<Model::Node_cptr> path = {start_node->parent()};
                auto lambda = [&](const auto &node, bool oc)
                {
                    const bool is_child = !path.empty() && node.parent() == path.back();

                    if (oc)
                    {
                        ++depth;
                        path.push_back(node.shared_from_this());
                    }
                    else
                    {
                        path.pop_back();
                        --depth;
                    }

                    if (!oc)
                        return true;

                    if (options_.depth >= 0 && depth > options_.depth+1)
                        return true;

                    std::cout << ' ';

                    //Aggregated
                    {
                        if (node.total_duration.as_minutes() == 0)
                            std::cout << "--------- ";
                        else
                            std::cout << std::setw(2) << node.total_duration << ' ';

                        if (node.total_duration.as_minutes() == 0 && node.total_todo.as_minutes() == 0)
                            std::cout << "--------- ";
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
                    }

                    std::cout << "| ";

                    //Node
                    {
                        if (node.duration)
                            std::cout << std::setw(2) << *node.duration << ' ';
                        else
                            std::cout << "--------- ";
                        if (node.todo)
                            std::cout << std::setw(2) << *node.todo << ' ';
                        else
                            std::cout << "--------- ";
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
                    }

                    std::cout << "| ";

                    //Tree
                    {
                        const auto tag = (!node.parent()) ? "<ROOT>" : node.tag;
                        if (is_child)
                            std::cout << std::string(depth*2, ' ') << tag << std::endl;
                        else
                            std::cout << std::string(depth*2, ' ') << Model::uri(node) << std::endl;
                    }

                    return true;
                };
                MSS(model_.traverse(start_node, lambda));
            }

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
