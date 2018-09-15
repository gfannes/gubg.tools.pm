#ifndef HEADER_pit_Model_hpp_ALREADY_INCLUDED
#define HEADER_pit_Model_hpp_ALREADY_INCLUDED

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

        struct Node
        {
            std::string tag;
            std::string note;
            std::optional<Moscow> moscow;
            double progress = 0.0;
            gubg::Army duration;
            std::optional<std::string> deadline;
            using Ptr = std::shared_ptr<Node>;
            using WPtr = std::weak_ptr<Node>;
            std::vector<Ptr> childs;
            std::map<std::string, WPtr> deps;

            Node(const std::string &tag): tag(tag) {}
        };

        bool load(const std::filesystem::path &filename)
        {
            MSS_BEGIN(bool, "");
            MSS(std::filesystem::is_regular_file(filename), std::cout << "Error: file " << filename << " does not exist" << std::endl);
            std::string content;
            MSS(gubg::file::read(content, filename));
            gubg::naft::Range range(content);
            root_.reset(new Node("/"));
            path_.push_back(root_);
            //Recursive parsing of content
            MSS(parse_(range), std::cout << "Error: parsing failed" << std::endl);
            MSS(range.empty(), std::cout << "Error: could not understand part \"" << range.str() << "\"" << std::endl);
            MSS_END();
        }

        template <typename Ftor>
        bool dfs(Ftor &&ftor) const
        {
            MSS_BEGIN(bool);
            MSS(!!root_);
            MSS(dfs_(*root_, ftor));
            MSS_END();
        }

    private:
        template <typename Ftor>
        bool dfs_(const Node &node, Ftor &&ftor) const
        {
            MSS_BEGIN(bool);
            MSS(ftor(node, true));
            for (const auto &child: node.childs)
            {
                MSS(dfs_(*child, ftor));
            }
            MSS(ftor(node, false));
            MSS_END();
        }

        bool parse_(gubg::naft::Range &range)
        {
            MSS_BEGIN(bool, "");
            std::string tag;
            while (range.pop_tag(tag))
            {
                L(C(tag));

                Node::Ptr ptr(new Node(tag));
                path_.back()->childs.push_back(ptr);
                path_.push_back(ptr);

                auto &node = *ptr;

                std::smatch m;

                gubg::naft::Attrs attrs; range.pop_attrs(attrs);
                for (const auto &p: attrs)
                {
                    const auto &key = p.first;
                    const auto &value = p.second;
                    L(C(key)C(value));
                    if (false) {}
                    else if (key == "note") {node.note = value;}
                    else if (key == "done") {node.progress = 1.0;}
                    else if (key == "deadline") {node.deadline = value;}
                    else if (key == "dep") {node.deps[value];}
                    else if (key == "must") {node.moscow = Moscow::Must;}
                    else if (key == "should") {node.moscow = Moscow::Should;}
                    else if (key == "could") {node.moscow = Moscow::Could;}
                    else if (key == "wont") {node.moscow = Moscow::Wont;}
                    else if (is_progress_(node.progress, key)) {}
                    else if (is_hours_(node.duration, key)) {}
                    else if (is_army_(node.duration, key)) {}
                    else
                    {
                        MSS(false, std::cout << "Error: unknown attribute key \"" << key << "\"" << std::endl);
                    }
                }
                gubg::naft::Range block;
                if (range.pop_block(block))
                    MSS(parse_(block));
                path_.pop_back();
            }
            MSS_END();
        }

        static bool is_progress_(double &progress, const std::string &key)
        {
            MSS_BEGIN(bool);
            gubg::Strange strange(key);
            double v;
            MSS_Q(strange.pop_float(v));
            MSS_Q(strange.pop_if('%'));
            MSS(strange.empty());
            progress = v/100.0;
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

        Node::Ptr root_;

        using Path = std::vector<Node::Ptr>;
        Path path_;

        const std::regex re_hours_{"\\d*h"};
    };
} 

#endif
