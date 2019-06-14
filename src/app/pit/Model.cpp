#include <pit/Model.hpp>

namespace pit { 

    namespace  { 

        constexpr const char *ns = nullptr;

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
        MSS_BEGIN(bool, ns);

        xtree_.clear();

        //Parse all the input files and add them to the xtree_
        for (const auto &input_file: input_files)
        {
            const std::filesystem::path filename = input_file.fn;
            log(Verbose) << "Loading " << input_file.fn << " and " << input_file.ns << "\n";

            MSS(std::filesystem::is_regular_file(filename), log(Error) << "Error: file " << filename << " does not exist" << std::endl);
            std::string content;
            MSS(gubg::file::read(content, filename));
            gubg::naft::Range range(content);

            //Recursive parsing of content
            Node *root = &xtree_.root();
            root->each_child([](const auto &ch){log(Debug) << ch << std::endl; return true;});
            const auto &ns = input_file.ns;
            if (!ns.empty())
            {
                MSS(root->each_child([&](const auto &child){return child.tag != ns;}), log(Error) << "Error: namespace \"" << ns << "\" is already in use" << std::endl);
                root = &add_child(*root, ns);
            }
            MSS(parse_(*root, range), log(Error) << "Error: parsing failed" << std::endl);
            MSS(range.empty(), log(Error) << "Error: could not understand part \"" << range.str().substr(0, 10) << "\"" << std::endl);
        }

        //Setup x-links
        {
            //Bring dependencies, belongs and sequential order into the system
            auto insert_links = [&](bool ok, auto &node)
            {
                MSS_BEGIN(bool);
                MSS(ok);
                for (const auto &dep: node.deps)
                {
                    std::string error;
                    auto to = resolve(dep, node, &error);
                    MSS(!!to, log(Error) << error << std::endl);
                    node.add_link(*to);
                }
                for (const auto &bel: node.belongs)
                {
                    std::string error;
                    auto from = resolve(bel, node, &error);
                    MSS(!!from, log(Error) << error << std::endl);
                    from->add_link(node);
                }
                if (node.sequential)
                {
                    Model::Node *prev = nullptr;
                    auto lambda = [&](auto &child)
                    {
                        if (prev)
                        {
                            log(Verbose) << "Adding link from " << child << " to " << *prev << std::endl;
                            child.add_link(*prev);
                        }
                        prev = &child;
                        return true;
                    };
                    MSS(node.each_child(lambda));
                }
                MSS_END();
            };
            MSS(xtree_.accumulate(true, insert_links));

            auto propagate_attributes = [](bool ok, auto &node)
            {
                MSS_BEGIN(bool);
                MSS(ok);

                const auto &parent_ptr = node.parent();
                if (parent_ptr)
                {
                    const auto &parent = *parent_ptr;
                    if (parent.done)
                        node.done = parent.done;
                }

                if (node.duration)
                    node.agg_duration = *node.duration;

                if (node.done)
                    node.todo.emplace();
                if (!node.todo)
                    node.todo = node.duration;
                if (node.todo)
                    node.agg_todo = *node.todo;

                if (node.ui_required_skill)
                {
                    node.required_skill = *node.ui_required_skill;
                }
                else
                {
                    if (parent_ptr)
                        node.required_skill = parent_ptr->required_skill;
                }
                MSS_END();
            };
            MSS(xtree_.accumulate(true, propagate_attributes));

            MSS(xtree_.process_xlinks([](const auto &cycle){log(Error) << "Error: cycle detected of size " << cycle.size() << "\n"; for (const auto &ptr: cycle) {log(Error) << *ptr << std::endl;}}));
        }

        MSS_END();
    }

    bool Model::plan(ResourceMgr &resource_mgr)
    {
        MSS_BEGIN(bool, ns);

        work_days_ = resource_mgr.work_days();

        int depth = 0;

        std::map<Model::Node *, bool> node__planned;

        std::list<std::string> errors;
        auto lambda = [&](Model::Node &node, bool oc, bool as_child){
            auto &planned = node__planned[&node];
            if (planned)
                return false;

            auto parent = node.parent();

            if (oc)
            {
                log(Verbose) << "Entering \"" << TagPath{node} << "\"\n";
                //Aggregate the "not_before" day from the parent
                if (parent)
                {
                    node.not_before = max(node.not_before, parent->not_before);
                    if (node.not_before)
                        log(Verbose) << "  * not_before: " << *node.not_before << "\n";
                }
                return true;
            }

            planned = true;

            auto todo_minutes = node.todo.value_or(gubg::Army{}).as_minutes();

            const auto &required_skill = node.required_skill;
            log(Verbose) << "Planning \"" << TagPath{node} << "\" for skill \"" << required_skill << "\" => " << todo_minutes << "min" << std::endl;

            auto aggregate_agg_last = [&](const auto &n){
                node.agg_last = max(node.agg_last, n.agg_last);
                return true;
            };
            node.each_child(aggregate_agg_last);
            node.each_sub(aggregate_agg_last);
            if (node.agg_last)
                log(Verbose) << "  * agg_last: " << *node.agg_last << std::endl;

            auto aggregate_into_parent = [&]()
            {
                if (parent)
                {
                    parent->agg_last = max(parent->agg_last, node.agg_last);
                    if (parent->sequential)
                        parent->not_before = max(parent->not_before, parent->agg_last);
                }
            };

            if (todo_minutes == 0)
            {
                aggregate_into_parent();
                return true;
            }

            auto start_day = Day::today();
            start_day = max(start_day, node.not_before);
            start_day = max(start_day, node.agg_last);
            log(Verbose) << "  * starting on " << start_day << std::endl;

            for (auto day = start_day; todo_minutes > 0; ++day)
            {
                if (!resource_mgr.is_in_range(day))
                {
                    std::ostringstream oss;
                    oss << "Error: could not plan task \"" << TagPath{node} << "\" requiring skill \"" << required_skill << "\": out of days." << std::endl;
                    errors.push_back(oss.str());
                    return false;
                }
                if (day.occupancy >= 0.9999)
                    continue;

                log(Debug) << "  " << day << " " << todo_minutes << "min left" << std::endl;
                Resources *resources = resource_mgr.get(day);
                if (!resources)
                    continue;

                Resource *ptr = nullptr;
                {
                    if (!node.worker)
                    {
                        //This task is not yet assigned to a worker: take the most efficient one that is available today
                        ptr = resources->get_most_efficient_for(required_skill);
                        if (!ptr)
                            continue;
                        auto &resource = *ptr;
                        log(Verbose) << "  * assigning to " << resource.name << std::endl;
                        log(Verbose) << "  * first day on " << day << std::endl;
                        node.worker = resource.name;
                        node.first = day;
                        node.agg_first = day;
                    }
                    else
                    {
                        //This task is already assigned to a worker: get that resource
                        ptr = resources->get_by_name(*node.worker);
                        if (!ptr)
                            continue;
                    }
                }
                auto &resource = *ptr;

                const auto minutes_per_day = 8*60;
                const auto efficiency = resource.efficiency(required_skill);
                const auto required = todo_minutes/efficiency/minutes_per_day;
                const auto available = std::min(1.0-resource.occupancy, 1.0-day.occupancy);
                if (available >= 0.0001)
                {
                    if (required <= available)
                    {
                        //We can finish the task today
                        resource.occupancy += required;
                        day.occupancy += required;
                        todo_minutes = 0;
                        log(Verbose) << "  * last day on " << day << std::endl;
                        node.last = day;
                        node.agg_last = day;
                        aggregate_into_parent();
                    }
                    else
                    {
                        //We cannot finish the task today
                        resource.occupancy += available;
                        int done_minutes = efficiency*minutes_per_day*available;
                        if (done_minutes == 0)
                            //Make sure we do not get stuck in the float-int conversions: keep making progress
                            done_minutes = 1;
                        todo_minutes -= done_minutes;
                    }
                }
            }

            return true;
        };
        xtree_.traverse(lambda, root());

        if (!errors.empty())
        {
            for (const auto &error: errors)
                log(Error) << error;
            MSS(false, log(Error) << errors.size() << " errors occured" << std::endl);
        }

        MSS_END();
    }

    bool Model::aggregate()
    {
        MSS_BEGIN(bool);

        //Aggregate totals
        {
            auto aggregator = [](auto &dst, const auto &src)
            {
                dst.agg_duration += src.agg_duration;
                dst.agg_todo += src.agg_todo;
                return true;
            };
            MSS(xtree_.aggregate(aggregator));
        }

        //Compute agg_first and agg_last
        {
            auto lambda = [](auto &node, bool oc, bool as_child){
                if (oc)
                    return true;
                auto lambda = [&](const auto &src){
                    if (!node.agg_first)
                        node.agg_first = src.agg_first;
                    else if (src.agg_first)
                        node.agg_first = std::min(*node.agg_first, *src.agg_first);

                    if (!node.agg_last)
                        node.agg_last = src.agg_last;
                    else if (src.agg_last)
                        node.agg_last = std::max(*node.agg_last, *src.agg_last);
                    return true;
                };
                node.each_child(lambda);
                node.each_sub(lambda);
                return true;
            };
            xtree_.traverse(lambda);
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
                else if (key == "name") {node.name = value;}
                else if (key == "note") {node.note = value;}
                else if (key == "todo")
                {
                    node.todo.emplace();
                    const auto ok = is_hours(*node.todo, value) || is_days(*node.todo, value) || is_army(*node.todo, value);
                    MSS(ok, log(Error) << "Error: \"todo\" should have a duration at " << position(range) << std::endl);
                }
                else if (key == "done") {node.done = true;}
                else if (key == "deadline") {node.deadline = value;}
                else if (key == "must") {node.moscow = Moscow::Must;}
                else if (key == "should") {node.moscow = Moscow::Should;}
                else if (key == "could") {node.moscow = Moscow::Could;}
                else if (key == "wont") {node.moscow = Moscow::Wont;}
                else if (key == "w") {}
                else if (key == "dep" || key == "d")
                {
                    std::string part;
                    for (gubg::Strange strange{value}; !strange.empty(); )
                    {
                        strange.pop_until(part, ',') || strange.pop_all(part);
                        node.deps.push_back(part);
                    }
                }
                else if (key == "belong" || key == "b" || key == "s")
                {
                    std::string part;
                    for (gubg::Strange strange{value}; !strange.empty(); )
                    {
                        strange.pop_until(part, ',') || strange.pop_all(part);
                        node.belongs.push_back(part);
                    }
                }
                else if (key == "skill") {node.ui_required_skill = value;}
                else if (key == "key" || key == "j") {node.key = value;}
                else if (key == "o") {node.sequential = true;}
                else if (key == "p" || key == "points") { node.duration.emplace().from_minutes(std::stol(value)*4*60); }
                else if (is_hours(duration, key) || is_days(duration, key) || is_army(duration, key)) { node.duration = duration; }
                else if (key.empty()) { }
                else
                {
                    MSS(false, log(Error) << "Error: unknown attribute key \"" << key << "\" at " << position(range) << std::endl);
                }
            }

            gubg::naft::Range block;
            if (range.pop_block(block))
            {
                MSS(parse_(node, block));
            }
        }
        MSS_END();
    }

} 
