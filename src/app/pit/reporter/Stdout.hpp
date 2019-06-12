#ifndef HEADER_pit_reporter_Stdout_hpp_ALREADY_INCLUDED
#define HEADER_pit_reporter_Stdout_hpp_ALREADY_INCLUDED

#include <pit/Options.hpp>
#include <pit/Model.hpp>
#include <gubg/xtree/Depth.hpp>
#include <optional>
#include <cmath>

namespace pit { namespace reporter { 

    class Stdout
    {
    public:
        bool process(const Options &options)
        {
            MSS_BEGIN(bool);
            before_depth_ = options.before_depth;
            after_depth_ = options.after_depth;
            MSS_END();
        }

        bool process(const Model &model, const Model::Node_cptr &start_node)
        {
            MSS_BEGIN(bool);

            std::cout << " Aggregated               | Node                     | Tree" << std::endl;
            std::cout << " Total     Todo      Prog | Total     Todo      Prog |" << std::endl;
            std::cout << "--------------------------|--------------------------|---------------------------------" << std::endl;

            std::list<Model::Node_cptr> path = {start_node->parent()};
            gubg::xtree::Depth depth;
            auto lambda = [&](const auto &node, bool oc, bool as_child)
            {
                gubg::xtree::Depth::Update update{depth, oc, as_child};
                if (before_depth_ && depth.before_x > *before_depth_)
                    return false;
                if (after_depth_ && depth.after_x > *after_depth_)
                    return false;

                const auto total_depth = depth.total();

                bool is_child;
                if (oc)
                {
                    is_child = !path.empty() && node.parent() == path.back();
                    path.push_back(node.shared_from_this());
                }
                else
                {
                    path.pop_back();
                    is_child = !path.empty() && node.parent() == path.back();
                }

                if (!oc)
                    return true;

                std::cout << ' ';

                //Aggregated
                {
                    if (node.agg_duration.as_minutes() == 0)
                        std::cout << "--------- ";
                    else
                        std::cout << std::setw(2) << node.agg_duration << ' ';

                    if (node.agg_duration.as_minutes() == 0 && node.agg_todo.as_minutes() == 0)
                        std::cout << "--------- ";
                    else
                        std::cout << std::setw(2) << node.agg_todo << ' ';

                    {
                        const double duration = node.agg_duration.as_minutes();
                        if (duration > 0)
                        {
                            const double todo = node.agg_todo.as_minutes();
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

                    if (node.agg_first && node.agg_last)
                        std::cout << *node.agg_first << " " << *node.agg_last;
                    else
                        std::cout << "---------- ----------";
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

                    if (node.first && node.last)
                        std::cout << *node.first << " " << *node.last;
                    else
                        std::cout << "---------- ----------";
                }

                std::cout << "| ";

                //Tree
                {
                    const auto tag = (!node.parent()) ? "<ROOT>" : node.tag;
                    if (is_child)
                        std::cout << std::string(total_depth*2, ' ') << tag << std::endl;
                    else
                        std::cout << std::string(total_depth*2, ' ') << TagPath(node) << std::endl;
                }

                return true;
            };
            model.traverse(start_node, lambda);

            MSS_END();
        }

    private:
        std::optional<unsigned int> before_depth_;
        std::optional<unsigned int> after_depth_;
    };

} } 

#endif
