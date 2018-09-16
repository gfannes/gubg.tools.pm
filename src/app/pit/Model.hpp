#ifndef HEADER_pit_Model_hpp_ALREADY_INCLUDED
#define HEADER_pit_Model_hpp_ALREADY_INCLUDED

#include "gubg/xtree/Model.hpp"
#include "gubg/mss.hpp"
#include "gubg/std/filesystem.hpp"
#include "gubg/naft/Range.hpp"
#include "gubg/file/System.hpp"
#include "gubg/std/optional.hpp"
#include "gubg/Army.hpp"
#include <string>
#include <iostream>
#include <memory>
#include <map>

namespace pit { 

    enum class Moscow { Must, Should, Could, Wont, };

    class Model
    {
    public:

        struct Data
        {
            std::string tag;
            std::string note;
            std::optional<Moscow> moscow;
            gubg::Army total_duration;
            gubg::Army total_todo;
            std::optional<gubg::Army> duration;
            std::optional<gubg::Army> todo;
            std::optional<std::string> deadline;

            Data(){}
            Data(const std::string &tag): tag(tag) {}
        };

        bool load(const std::filesystem::path &filename)
        {
            MSS_BEGIN(bool, "");
            MSS(std::filesystem::is_regular_file(filename), std::cout << "Error: file " << filename << " does not exist" << std::endl);
            std::string content;
            MSS(gubg::file::read(content, filename));
            gubg::naft::Range range(content);
            xtree_.clear();
            //Recursive parsing of content
            MSS(parse_(xtree_.root(), range), std::cout << "Error: parsing failed" << std::endl);
            MSS(range.empty(), std::cout << "Error: could not understand part \"" << range.str().substr(0, 10) << "\"" << std::endl);
            MSS_END();
        }

        template <typename Ftor>
        bool dfs(Ftor &&ftor) const
        {
            return xtree_.accumulate(true, [&](bool ok, const auto &node){ftor(node); return true;});
        }

    private:
        using XTree = gubg::xtree::Model<Data>;

        bool parse_(XTree::Node &parent, gubg::naft::Range &range)
        {
            MSS_BEGIN(bool, "");

            std::string tag;
            while (range.pop_tag(tag))
            {
                L(C(tag));

                auto &node = parent.emplace_back(tag);

                gubg::Army duration;

                gubg::naft::Attrs attrs; range.pop_attrs(attrs);
                for (const auto &p: attrs)
                {
                    const auto &key = p.first;
                    const auto &value = p.second;
                    L(C(key)C(value));

                    if (false) {}
                    else if (key == "note") {node.note = value;}
                    else if (key == "todo")
                    {
                        node.todo.emplace();
                        MSS(is_hours_(*node.todo, value) || is_days_(*node.todo, value) || is_army_(*node.todo, value));
                    }
                    else if (key == "done") {node.todo.emplace();}
                    else if (key == "deadline") {node.deadline = value;}
                    else if (key == "must") {node.moscow = Moscow::Must;}
                    else if (key == "should") {node.moscow = Moscow::Should;}
                    else if (key == "could") {node.moscow = Moscow::Could;}
                    else if (key == "wont") {node.moscow = Moscow::Wont;}
                    else if (key == "who") {}
                    else if (key == "dep") {}
                    else if (is_hours_(duration, key) || is_days_(duration, key) || is_army_(duration, key)) {node.duration = duration;}

                    else if (key.empty())
                    {
                    }

                    else
                    {
                        MSS(false, std::cout << "Error: unknown attribute key \"" << key << "\"" << std::endl);
                    }
                }

                if (node.duration)
                    node.total_duration = *node.duration;
                if (node.todo)
                    node.total_todo = *node.todo;
                else if (node.duration)
                    node.total_todo = *node.duration;

                gubg::naft::Range block;
                if (range.pop_block(block))
                {
                    MSS(parse_(node, block));
                }
                parent.total_duration += node.total_duration;
                parent.total_todo += node.total_todo;
            }
            MSS_END();
        }

        static bool is_hours_(gubg::Army &duration, const std::string &key)
        {
            MSS_BEGIN(bool);
            gubg::Strange strange(key);
            double hours;
            MSS_Q(strange.pop_float(hours));
            MSS_Q(strange.pop_if('h'));
            MSS(strange.empty());
            duration.from_minutes(hours*60.0);
            MSS_END();
        }
        static bool is_days_(gubg::Army &duration, const std::string &key)
        {
            MSS_BEGIN(bool);
            gubg::Strange strange(key);
            double days;
            MSS_Q(strange.pop_float(days));
            MSS_Q(strange.pop_if('d'));
            MSS(strange.empty());
            duration.from_minutes(days*8.0*60.0);
            MSS_END();
        }
        static bool is_army_(gubg::Army &duration, const std::string &key)
        {
            MSS_BEGIN(bool);
            gubg::Strange strange(key);
            unsigned int v;
            MSS_Q(strange.pop_decimal(v));
            MSS(strange.empty());
            duration.from_army(v);
            MSS_END();
        }

        XTree xtree_;
    };
} 

#endif
