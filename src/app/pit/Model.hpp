#ifndef HEADER_pit_Model_hpp_ALREADY_INCLUDED
#define HEADER_pit_Model_hpp_ALREADY_INCLUDED

#include <pit/Types.hpp>
#include <gubg/xtree/Model.hpp>
#include <gubg/mss.hpp>
#include <gubg/std/filesystem.hpp>
#include <gubg/naft/Range.hpp>
#include <gubg/file/System.hpp>
#include <gubg/std/optional.hpp>
#include <gubg/string_algo/algo.hpp>
#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <list>

namespace pit { 

    class TagPath: public std::vector<Tag>
    {
    public:
        static const char sep() {return '/';}

        template <typename Node>
        TagPath(const Node &node)
        {
            for (const Node *ptr = &node; ptr; ptr = ptr->parent().get())
                push_back(ptr->tag);
            std::reverse(RANGE(*this));
        }
        TagPath(const std::string &str)
        {
            gubg::string_algo::split(*this, str, sep());
        }
        TagPath(const TagPath &a, const TagPath &b)
        {
            insert(end(), RANGE(a));
            insert(end(), RANGE(b));
        }

        void stream(std::ostream &os) const
        {
            gubg::OnlyOnce skip_sep;
            for (const auto &e: *this)
            {
                if (!skip_sep())
                    os << sep();
                os << e;
            }
        }
        std::string str() const
        {
            oss_.str("");
            stream(oss_);
            return oss_.str();
        }
    private:
        static std::ostringstream oss_;
    };
    inline std::ostream &operator<<(std::ostream &os, const TagPath &tp)
    {
        tp.stream(os);
        return os;
    }

    class Model
    {
    public:
        using XTree = gubg::xtree::Model<Data>;
        using Node = typename XTree::Node;
        using Node_ptr = typename XTree::Node_ptr;
        using Node_cptr = typename XTree::Node_cptr;

        bool load(const InputFiles &input_files)
        {
            MSS_BEGIN(bool);

            xtree_.clear();

            for (const auto &input_file: input_files)
            {
                const std::filesystem::path filename = input_file.fn;

                MSS(std::filesystem::is_regular_file(filename), std::cout << "Error: file " << filename << " does not exist" << std::endl);
                std::string content;
                MSS(gubg::file::read(content, filename));
                gubg::naft::Range range(content);

                //Recursive parsing of content
                Node *root = &xtree_.root();
                const auto &ns = input_file.ns;
                if (!ns.empty())
                {
                    MSS(root->each_child([&](const auto &child){return child.tag != ns;}), std::cout << "Error: namespace \"" << ns << "\" is already in use" << std::endl);
                    root = &add_child(*root, ns);
                }
                MSS(parse_(*root, range), std::cout << "Error: parsing failed" << std::endl);
                MSS(range.empty(), std::cout << "Error: could not understand part \"" << range.str().substr(0, 10) << "\"" << std::endl);
            }

            //Setup x-links
            {
                auto insert_links = [&](bool ok, auto &node)
                {
                    for (const auto &dep: node.deps)
                    {
                        std::string error;
                        auto to = resolve(dep, node, &error);
                        MSS(!!to, std::cout << error << std::endl);
                        node.add_link(*to);
                    }
                    return true;
                };
                MSS(xtree_.accumulate(true, insert_links));
                MSS(xtree_.process_xlinks([](const auto &node, const auto &from, const auto &msg){std::cout << "Error: problem detected for node " << TagPath(node) << " from " << TagPath(from) << ": " << msg << std::endl;}));
            }

            //Aggregate totals
            {
                auto add_totals = [](auto &dst, const auto &src)
                {
                    dst.total_duration += src.total_duration;
                    dst.total_todo += src.total_todo;
                    return true;
                };
                MSS(xtree_.aggregate(add_totals));
            }

            MSS_END();
        }

        Node_cptr root() const {return xtree_.root_ptr();}

        template <typename Ftor>
        bool traverse(const Node_cptr &node, Ftor &&ftor, bool show_xlinks) const
        {
            MSS_BEGIN(bool);
            MSS(!!node);
            MSS(xtree_.traverse(ftor, show_xlinks, node));
            MSS_END();
        }

        std::string str() const
        {
            std::ostringstream oss;
            oss << "[model]{\n";
            auto lambda = [&](bool ok, const Node &node){
                oss << "  [node](path:" << TagPath{node} << "){\n";
                if (false)
                    node.each_in([&](const Node &n){oss << "    [in](path:" << TagPath{n} << ")\n"; return true;});
                if (true)
                    node.each_out([&](const Node &n){oss << "    [out](path:" << TagPath{n} << ")\n"; return true;});
                if (true)
                    node.each_sub([&](const Node &n){oss << "    [sub](path:" << TagPath{n} << ")\n"; return true;});
                oss << "  }\n";
                return ok;
            };
            xtree_.accumulate(true, lambda);
            oss << "}\n";
            return oss.str();
        }

        Node_ptr resolve(const std::string &dep, const Node &from, std::string *error = nullptr)
        {
            TagPath dep_tp{dep};

            for (TagPath tp{from}; true; tp.pop_back())
            {
                const TagPath combined_tp{tp, dep_tp};

                auto it = tagpath__node_.find(combined_tp);
                if (it != tagpath__node_.end())
                    return it->second;
                if (tp.empty())
                    break;
            }
            
            if (!!error)
            {
                std::ostringstream oss;
                oss << "Error: could not resolve dependency \"" << dep << "\" for " << TagPath(from);
                *error = oss.str();
            }
            return Node_ptr{};
        }

        Node &add_child(Node &parent, const Tag &tag)
        {
            auto &node = parent.emplace_back(tag);
            tagpath__node_[TagPath{node}] = node.shared_from_this();
            return node;
        }

    private:
        bool parse_(Node &parent, gubg::naft::Range &range)
        {
            MSS_BEGIN(bool);

            Tag tag;
            while (range.pop_tag(tag))
            {
                L(C(tag));

                auto &node = add_child(parent, tag);

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
                        const auto ok = is_hours_(*node.todo, value) || is_days_(*node.todo, value) || is_army_(*node.todo, value);
                        MSS(ok, std::cout << "Error: \"todo\" should have a duration at " << position_(range) << std::endl);
                    }
                    else if (key == "done") {node.todo.emplace();}
                    else if (key == "deadline") {node.deadline = value;}
                    else if (key == "must") {node.moscow = Moscow::Must;}
                    else if (key == "should") {node.moscow = Moscow::Should;}
                    else if (key == "could") {node.moscow = Moscow::Could;}
                    else if (key == "wont") {node.moscow = Moscow::Wont;}
                    else if (key == "s") {node.story = value;}
                    else if (key == "w") {}
                    else if (key == "dep") {node.deps.push_back(value);}
                    else if (is_hours_(duration, key) || is_days_(duration, key) || is_army_(duration, key)) {node.duration = duration;}

                    else if (key.empty())
                    {
                    }

                    else
                    {
                        MSS(false, std::cout << "Error: unknown attribute key \"" << key << "\" at " << position_(range) << std::endl);
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
            }
            MSS_END();
        }

        static std::string position_(const gubg::naft::Range &range)
        {
            std::ostringstream oss;
            auto pos = range.position();
            oss << "line " << pos.line+1 << ", column " << pos.column+1;
            return oss.str();
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
        std::map<TagPath, Node_ptr> tagpath__node_;
    };
} 

#endif
