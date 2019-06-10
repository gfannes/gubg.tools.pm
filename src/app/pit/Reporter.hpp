#ifndef HEADER_pit_Reporter_hpp_ALREADY_INCLUDED
#define HEADER_pit_Reporter_hpp_ALREADY_INCLUDED

#include <pit/Options.hpp>
#include <pit/Model.hpp>
#include <optional>
#include <cmath>

namespace pit { 

    class Reporter
    {
    public:
        bool process(const Options &options)
        {
            MSS_BEGIN(bool);
            uri_ = options.uri;
            depth_ = options.depth;
            show_xlinks_ = options.show_xlinks;
            MSS_END();
        }
        bool process(const Model &model)
        {
            MSS_BEGIN(bool);

            auto start_node = model.root();
            if (uri_)
            {
                const auto &uri = *uri_;
                std::string error;
                auto ptr = model.resolve(uri, *start_node, &error);
                MSS(!!ptr, std::cout << "Error: could not find node " << uri << ": " << error << std::endl);
                start_node = ptr;
            }

            std::cout << " Aggregated               | Node                     | Tree" << std::endl;
            std::cout << " Total     Todo      Prog | Total     Todo      Prog |" << std::endl;
            std::cout << "--------------------------|--------------------------|---------------------------------" << std::endl;

            unsigned int depth = 0;
            std::list<Model::Node_cptr> path = {start_node->parent()};
            unsigned int xcount = 0;
            auto lambda = [&](const auto &node, bool oc)
            {
                bool is_child;
                if (oc)
                {
                    ++depth;
                    is_child = !path.empty() && node.parent() == path.back();
                    path.push_back(node.shared_from_this());
                    if (!is_child)
                        ++xcount;
                }
                else
                {
                    path.pop_back();
                    is_child = !path.empty() && node.parent() == path.back();
                    if (!is_child)
                        --xcount;
                    --depth;
                }

                /* std::cout << TagPath{node} << " " << C(oc)C(path.size())C(is_child)C(xcount) << std::endl; */

                if (!oc)
                    return true;

                if (depth_ && depth > *depth_+1)
                    return true;

                if (!show_xlinks_ && xcount > 0)
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

                    if (node.agg_first)
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

                    std::cout << node.first << " " << node.last;
                }

                std::cout << "| ";

                //Tree
                {
                    const auto tag = (!node.parent()) ? "<ROOT>" : node.tag;
                    if (is_child)
                        std::cout << std::string(depth*2, ' ') << tag << std::endl;
                    else
                        std::cout << std::string(depth*2, ' ') << TagPath(node) << std::endl;
                }

                return true;
            };
            MSS(model.traverse(start_node, lambda, show_xlinks_));

            MSS_END();
        }

    private:
        std::optional<std::string> uri_;
        std::optional<unsigned int> depth_;
        bool show_xlinks_ = false;
    };

} 

#endif
