#include <pit/Model.hpp>

namespace pit { 

    namespace  { 
        std::string position(const gubg::naft::Range &range)
        {
            std::ostringstream oss;
            auto pos = range.position();
            oss << "line " << pos.line+1 << ", column " << pos.column+1;
            return oss.str();
        }

        bool is_hours(gubg::Army &duration, const std::string &key)
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
        bool is_days(gubg::Army &duration, const std::string &key)
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
        bool is_army(gubg::Army &duration, const std::string &key)
        {
            MSS_BEGIN(bool);
            gubg::Strange strange(key);
            unsigned int v;
            MSS_Q(strange.pop_decimal(v));
            MSS(strange.empty());
            duration.from_army(v);
            MSS_END();
        }
    } 

    bool Model::load(const InputFiles &input_files)
    {
        MSS_BEGIN(bool);

        xtree_.clear();

        //Parse all the input files and add them to the xtree_
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
                MSS_BEGIN(bool);
                MSS(ok);
                for (const auto &dep: node.deps)
                {
                    std::string error;
                    auto to = resolve(dep, node, &error);
                    MSS(!!to, std::cout << error << std::endl);
                    node.add_link(*to);
                }
                if (node.sequential)
                {
                    Model::Node *prev = nullptr;
                    auto lambda = [&](auto &child)
                    {
                        if (prev)
                            child.add_link(*prev);
                        prev = &child;
                        return true;
                    };
                    MSS(node.each_child(lambda));
                }
                MSS_END();
            };
            MSS(xtree_.accumulate(true, insert_links));
            auto propagate = [](bool ok, auto &node)
            {
                MSS_BEGIN(bool);
                MSS(ok);
                if (node.ui_required_skill)
                {
                    node.required_skill = *node.ui_required_skill;
                }
                else
                {
                    const auto &parent = node.parent();
                    if (parent)
                        node.required_skill = parent->required_skill;
                }
                MSS_END();
            };
            MSS(xtree_.accumulate(true, propagate));
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

    std::string Model::str() const
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

    Model::Node_ptr Model::resolve(const std::string &dep, const Node &from, std::string *error) const
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

    Model::Node &Model::add_child(Node &parent, const Tag &tag)
    {
        auto &node = parent.emplace_back(tag);
        tagpath__node_[TagPath{node}] = node.shared_from_this();
        return node;
    }

    bool Model::parse_(Node &parent, gubg::naft::Range &range)
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
                    const auto ok = is_hours(*node.todo, value) || is_days(*node.todo, value) || is_army(*node.todo, value);
                    MSS(ok, std::cout << "Error: \"todo\" should have a duration at " << position(range) << std::endl);
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
                else if (key == "skill") {node.ui_required_skill = value;}
                else if (key == "o") {node.sequential = true;}
                else if (is_hours(duration, key) || is_days(duration, key) || is_army(duration, key))
                {
                    node.duration = duration;
                    node.todo = duration;
                }
                else if (key.empty())
                {
                }

                else
                {
                    MSS(false, std::cout << "Error: unknown attribute key \"" << key << "\" at " << position(range) << std::endl);
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

} 
