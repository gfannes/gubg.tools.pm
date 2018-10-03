#ifndef HEADER_pit_Model_hpp_ALREADY_INCLUDED
#define HEADER_pit_Model_hpp_ALREADY_INCLUDED

#include "gubg/xtree/Model.hpp"
#include "gubg/mss.hpp"
#include "gubg/std/filesystem.hpp"
#include "gubg/naft/Range.hpp"
#include "gubg/file/System.hpp"
#include "gubg/std/optional.hpp"
#include "gubg/Army.hpp"
#include "gubg/string_algo/algo.hpp"
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <list>

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
            std::string story;
            std::list<std::string> deps;

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

            //Setup x-links
            {
                auto insert_links = [&](bool ok, auto &node)
                {
                    for (const auto &dep: node.deps)
                    {
                        std::cout << uri_(node) << " => " << dep << std::endl;
                        std::string error;
                        auto to = resolve_(dep, node, &error);
                        MSS(!!to, std::cout << error << std::endl);
                        node.add_link(*to);
                    }
                    return true;
                };
                MSS(xtree_.accumulate(true, insert_links));
                MSS(xtree_.process_xlinks([](const auto &node, const auto &from, const auto &msg){std::cout << "Error: problem detected for node " << uri_(node) << " from " << uri_(from) << ": " << msg << std::endl;}));
            }

            MSS_END();
        }

        template <typename Ftor>
        bool dfs(Ftor &&ftor) const
        {
            return xtree_.accumulate(true, [&](bool ok, const auto &node){ftor(node); return true;});
        }

    private:
        using XTree = gubg::xtree::Model<Data>;
        using Node = typename XTree::Node;
        using Node_ptr = typename XTree::Node_ptr;

        Node_ptr resolve_(const std::string &dep, const Node &node, std::string *error = nullptr)
        {
            S("");
            std::list<std::string> parts;
            Node_ptr res = xtree_.root_ptr();
            gubg::string_algo::split(parts, dep, '/');
            for (const auto &part: parts)
            {
                auto &n = *res;
                res.reset();
                n.each_child([&](auto &child){if (child.tag == part) {res = child.shared_from_this(); return false;} return true;});
                if (!res)
                {
                    if (!!error)
                    {
                        std::ostringstream oss;
                        oss << "Error: could not find part \"" << part << "\" from " << uri_(n) << " for " << uri_(node);
                        *error = oss.str();
                    }
                    break;
                }
            }
            return res;
        }
        static std::string uri_(const Node &node)
        {
            typename XTree::Path path;
            node.path(path);
            std::ostringstream oss;
            for (const auto &n: path)
                oss << n->tag << "/";
            return oss.str();
        }

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
                    else if (key == "s") {node.story = value;}
                    else if (key == "who") {}
                    else if (key == "dep") {node.deps.push_back(value);}
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
