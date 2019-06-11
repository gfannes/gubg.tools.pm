#ifndef HEADER_pit_reporter_Tsv_hpp_ALREADY_INCLUDED
#define HEADER_pit_reporter_Tsv_hpp_ALREADY_INCLUDED

#include <pit/Model.hpp>
#include <pit/Options.hpp>
#include <string>
#include <fstream>

namespace pit { namespace reporter { 

    class Tsv
    {
    public:
        bool process(const Options &options)
        {
            MSS_BEGIN(bool);
            MSS(!!options.output_fn);
            fo_.open(*options.output_fn);
            MSS(fo_.good());
            MSS_END();
        }

        bool process(const Model &model, const Model::Node_cptr &start_node)
        {
            MSS_BEGIN(bool);

            const auto &work_days = model.work_days();

            add_("tag");
            add_("path");
            add_("duration (days)");
            add_("todo (days)");
            add_("agg_duration (days)");
            add_("agg_todo (days)");
            add_("first");
            add_("last");
            add_("agg_first");
            add_("agg_last");
            add_("worker");
            for (const auto &day: work_days)
                add_(day);
            newline_();

            auto lambda = [&](auto &node, bool oc){
                if (!oc)
                    return true;
                add_(node.tag);
                add_(TagPath{node});
                auto as_hours = [](const auto &army){return double(army.as_minutes())/60.0/8.0;};
                add_(node.duration ?  as_hours(*node.duration) : 0);
                add_(node.todo ?  as_hours(*node.todo) : 0);
                add_(as_hours(node.agg_duration));
                add_(as_hours(node.agg_todo));
                add_(node.first, "");
                add_(node.last, "");
                add_(node.agg_first, "");
                add_(node.agg_last, "");
                add_(node.worker, "");
                for (const auto &day: work_days)
                {
                    if (node.first && node.last)
                    {
                        if (false) {}
                        else if (*node.first <= day && day <= *node.last)
                            add_(0.5);
                        else if (*node.agg_first <= day && day <= *node.agg_last)
                            add_(0.0);
                        else if (day < *node.agg_first)
                            add_("");
                        else
                            add_(1.0);
                    }
                    else if (node.agg_first && node.agg_last)
                    {
                        if (false) {}
                        else if (*node.agg_first <= day && day <= *node.agg_last)
                            add_(0.0);
                        else if (day < *node.agg_first)
                            add_("");
                        else
                            add_(1.0);
                    }
                    else
                        add_("");
                    /* if (node.agg_first && day < *node.agg_first) */
                    /*     add_(""); */
                    /* else if (node.first && day < *node.first) */
                    /*     add_(0.0); */
                    /* else if (node.last && day < *node.last) */
                    /*     add_(0.5); */
                    /* else if (node.agg_last && day < *node.agg_last) */
                    /*     add_(0.0); */
                    /* else */
                    /*     add_(1.0); */
                }
                newline_();
                return true;
            };
            MSS(model.traverse(start_node, lambda, true));

            MSS_END();
        }

    private:
        template <typename T>
        void add_(const T &v)
        {
            if (!is_new_line_)
            {
                fo_ << '\t';
            }
            is_new_line_ = false;
            fo_ << v;
        }
        template <typename T, typename D>
        void add_(const std::optional<T> &opt, const D &d)
        {
            if (opt)
                add_(*opt);
            else
                add_(d);
        }
        void newline_()
        {
            fo_ << '\n';
            is_new_line_ = true;
        }

        bool is_new_line_ = true;
        std::ofstream fo_;
    };

} } 

#endif
