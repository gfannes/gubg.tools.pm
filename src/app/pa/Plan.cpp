#include "pa/Plan.hpp"
#include "pa/Planner.hpp"
#include "gubg/xml/Builder.hpp"
#include "gubg/Range.hpp"
#include <string>
#include <fstream>
#include <map>
#include <set>
#include <functional>
using namespace pa;
using namespace std;
using namespace gubg::xml::builder;

namespace  { 
    const char *logns = "pa::Plan";
    enum class Format
    {
        Text, Html, Timeline,
    };
} 

    template <typename Task>
string taskDescription(const Task &task)
{
    ostringstream oss;
    oss << task.fullName() << "(" << task.cumulSweat << ")";
    return oss.str();
}
bool stream(ostream &os, Planner &planner, Plan::View view, Format format)
{
    MSS_BEGIN(bool);
    using namespace gubg::planning;
    switch (view)
    {
        case Plan::Overview:
            switch (format)
            {
                case ::Format::Text:
                    {
                        size_t maxLen = 0;
                        for (auto p: planner.root->tasksPerDeadline())
                        {
                            const auto descr = taskDescription(*p.second);
                            if (descr.size() > maxLen)
                                maxLen = descr.size();
                        }
                        for (auto p: planner.root->tasksPerDeadline())
                        {
                            auto &task = *p.second;
                            const auto descr = taskDescription(*p.second);
                            os << descr << string(maxLen-descr.size()+1, ' ') << task.start << " -- " << task.stop;
                            if (task.stop > *p.first)
                                os << " !";
                            else
                                os << "  ";
                            os << "deadline: " << *p.first << endl;
                        }
                    }
                    break;
                case ::Format::Html:
                    {
                        Tag html(os, "html");
                        auto body = html.tag("body");
                        body.tag("h1") << "Planning overview on " << Day::today();
                        {
                            auto table = body.tag("table");
                            {
                                auto tr = table.tag("tr");
                                tr.tag("th") << "Task";
                                tr.tag("th") << "Start";
                                tr.tag("th") << "Stop";
                                tr.tag("th") << "Deadline";
                            }
                            for (auto p: planner.root->tasksPerDeadline())
                            {
                                auto &task = *p.second;
                                const auto descr = taskDescription(*p.second);
                                auto tr = table.tag("tr");
                                tr.tag("td") << descr;
                                tr.tag("td") << task.start;
                                tr.tag("td") << task.stop;
                                if (task.stop > *p.first)
                                {
                                    tr.tag("td").tag("font").attr("color", "red") << *p.first;
                                }
                                else
                                {
                                    tr.tag("td") << *p.first;
                                }
                            }
                        }
                    }
                    break;
            }
            break;
        case Plan::Products:
            {
                S(logns);
                auto leafTasks = gubg::tree::dfs::leafs(*planner.root);
                using StartStop = pair<Day, Day>;
                using StartStopPerNameParts = map<Task::NameParts, StartStop>;
                StartStopPerNameParts startstop_per_nameparts;
                StartStop period;
                using StartStopPerProduct = map<string, StartStop>;
                using StartStopPerProductPerLine = map<string ,StartStopPerProduct>;
                StartStopPerProductPerLine startstop_per_product_per_line;
                auto enlarge = [](StartStop &ss, const Day &start_, const Day &stop_)
                {
                    {
                        auto &start = ss.first;
                        if (!start.is_valid() || start_ < start)
                            start = start_;
                    }
                    {
                        auto &stop = ss.second;
                        if (stop < stop_)
                            stop = stop_;
                    }
                };
                auto name_from_parts = [](const Task::NameParts &parts)
                {
                    ostringstream oss;
                    for (const auto &part: parts)
                        oss << '/' << part;
                    return oss.str();
                };
                size_t max_line_size = 0;
                for (auto leaf: leafTasks)
                {
                    auto &task = *leaf;

                    {
                        auto parts = task.name_parts();
                        if (!parts.empty())
                        {
                            parts.pop_front();
                            for (; !parts.empty(); parts.pop_back())
                            {
                                enlarge(startstop_per_nameparts[parts], task.start, task.stop);
                                L(C(name_from_parts(parts))C(task.start)C(task.stop));
                            }
                        }
                    }

                    const unsigned int line_level = 3;
                    const auto line = task.base_name(line_level);
                    max_line_size = max(max_line_size, line.size());
                    const unsigned int product_level = 4;
                    const auto product = task.base_name(product_level, line_level);
                    enlarge(startstop_per_product_per_line[line][product], task.start, task.stop);
                    enlarge(period, task.start, task.stop);
                }
                using ProductsPerStop = multimap<Day, string>;
                ProductsPerStop products_per_stop;
                for (const auto &p: startstop_per_product_per_line)
                {
                    const auto &line = p.first;
                    for (const auto &pp: p.second)
                    {
                        const auto &product = pp.first;
                        const auto &stop = pp.second.second;
                        products_per_stop.emplace(stop, line+": "+string(max_line_size-line.size(), ' ')+product);
                    }
                }
                switch (format)
                {
                    case ::Format::Text:
                        {
                            size_t max_product_size = 0;
                            for (const auto &p: products_per_stop)
                            {
                                const auto &stop = p.first;
                                const auto &product = p.second;
                                max_product_size = max(product.size(), max_product_size);
                            }
                            os << "# Product delivery overview on " << Day::today() << endl << endl;
                            for (const auto &p: products_per_stop)
                            {
                                const auto &stop = p.first;
                                const auto &product = p.second;
                                assert(product.size() <= max_product_size);
                                os << product << string(max_product_size-product.size(), ' ') << " => ETA: " << stop << endl;
                            }
                        }
                        break;
                    case ::Format::Html:
                        {
                            Tag html(os, "html");
                            auto body = html.tag("body");
                            body.tag("h1") << "Product delivery overview on " << Day::today();
                            {
                                auto table = body.tag("table");
                                {
                                    auto tr = table.tag("tr");
                                    tr.tag("th") << "Product";
                                    tr.tag("th") << "ETA";
                                }
                                for (const auto &p: products_per_stop)
                                {
                                    const auto &stop = p.first;
                                    const auto &product = p.second;
                                    auto tr = table.tag("tr");
                                    tr.tag("td") << product;
                                    tr.tag("td") << stop;
                                }
                            }
                        }
                        break;
                    case ::Format::Timeline:
                        {
                            {
                                Header header(os);
                                header.attr("version", "1.0");
                                header.attr("encoding", "utf-8");
                            }
                            Tag timeline(os, "timeline");
                            timeline.tag("version") << "1.10.0 (1043763d680f 2016-04-30)";
                            timeline.tag("timetype") << "gregoriantime";

                            auto category_name = [](size_t depth)
                            {
                                ostringstream oss; oss << "depth_" << depth;
                                return oss.str();
                            };

                            {
                                auto categories_tag = timeline.tag("categories");
                                size_t max_name_parts = 0;
                                for (const auto &p: startstop_per_nameparts)
                                    max_name_parts = max(max_name_parts, p.first.size());

                                auto color = [&](float depth)
                                {
                                    depth -= 1;//0-based now
                                    ostringstream oss;
                                    const float factor = depth/float(max_name_parts-1);
                                    oss << int(255.0*(1.0-factor)) << ',' << int(255.0*factor) << ',' << int(0*(1.0-factor));
                                    return oss.str();
                                };

                                for (size_t depth = 1; depth <= max_name_parts; ++depth)
                                {
                                    auto cat = categories_tag.tag("category");
                                    cat.tag("name") << category_name(depth);
                                    cat.tag("color") << color(depth);
                                    cat.tag("progress_color") << "255,80,80";
                                    cat.tag("done_color") << "255,80,80";
                                    cat.tag("font_color") << "0,0,0";
                                }
                            }

                            auto format_time = [](const Day &day, bool is_start)
                            {
                                ostringstream oss;
                                oss << day.year() << '-' << day.month() << '-' << day.day();
                                oss << (is_start ? " 14:00:00" : " 10:00:00");
                                return oss.str();
                            };

                            {
                                auto events = timeline.tag("events");
                                for (const auto &p: startstop_per_nameparts)
                                {
                                    const auto &parts = p.first;
                                    const auto &ss = p.second;

                                    //We only show the tasks that take at least one day.
                                    //Else, format_time() will generate start date after the stop date
                                    if (ss.first != ss.second)
                                    {
                                        auto event = events.tag("event");
                                        event.tag("start") << format_time(ss.first, true);
                                        event.tag("end") << format_time(ss.second, false);
                                        event.tag("text") << parts.back();
                                        event.tag("progress") << 0;
                                        event.tag("fuzzy") << "False";
                                        event.tag("locked") << "False";
                                        event.tag("ends_today") << "False";
                                        event.tag("category") << category_name(parts.size());
                                        event.tag("default_color") << "200,200,200";
                                    }
                                }
                            }
                            {
                                auto view = timeline.tag("view");
                                {
                                    auto displayed_period = view.tag("displayed_period");
                                    displayed_period.tag("start") << format_time(period.first, true);
                                    displayed_period.tag("end") << format_time(period.second, false);
                                }
                                {
                                    auto hidden_categories = view.tag("hidden_categories");
                                }
                            }
                        }
                        break;
                }
            }
            break;
        case Plan::Details:
            switch (format)
            {
                case ::Format::Text: planner.planning.stream(os, gubg::planning::Format::Text); break;
                case ::Format::Html: planner.planning.stream(os, gubg::planning::Format::Html); break;
            }
            break;
        case Plan::Resource:
            {
                std::unique_ptr<Tag> html;
                std::unique_ptr<Tag> body;
                std::unique_ptr<Tag> table;
                switch (format)
                {
                    case ::Format::Text:
                        os << "# Resource planning per worker on " << Day::today() << endl << endl;
                        break;
                    case ::Format::Html:
                        {
                            html.reset(new Tag(os, "html"));
                            body.reset(new Tag(html->tag("body")));
                            MSS(!!body);
                            body->tag("h1") << "Resource planning per worker on " << Day::today();
                        }
                        break;
                }

                Day first_day, last_day;
                MSS(planner.planning.get_first_day(first_day));
                MSS(planner.planning.get_last_day(last_day));
                const auto all_days = day_range(first_day, last_day);

                const auto workers = planner.planning.workers();
                for (const auto &worker: workers)
                {
                    switch (format)
                    {
                        case ::Format::Text:
                            os << endl << endl << "## Monthly planned overview for " << worker << ":" << endl;
                            break;
                        case ::Format::Html:
                            MSS(!!body);
                            body->tag("h2") << "Monthly planned overview for " << worker;
                            break;
                    }

                    auto all_days_range = gubg::make_range(RANGE(all_days));
                    L(all_days_range.size());
                    while (!all_days_range.empty())
                    {
                        Day first_day_of_month = all_days_range.front();
                        std::set<Day> days;
                        {
                            for (; !all_days_range.empty(); all_days_range.pop_front())
                            {
                                const auto &day = all_days_range.front();
                                if (day.year() != first_day_of_month.year() || day.month() != first_day_of_month.month())
                                    break;
                                days.insert(day);
                            }
                        }

                        Planning::SweatPerTask sweat_per_task;
                        planner.planning.get_sweat_per_task(sweat_per_task, worker, days, 3);

                        Planning::SweatPerTask sweat_per_task_cumul;
                        std::vector<Task::Ptr> ordered_prods;
                        double total_sweat = 0.0;
                        size_t max_name_size = 0;
                        for (const auto &t_s: sweat_per_task)
                        {
                            const auto sweat = t_s.second;
                            total_sweat += sweat;
                            auto task = t_s.first;
                            MSS(!!task);
                            ordered_prods.push_back(task);
                            max_name_size = std::max(max_name_size, task->fullName().size());
                            for (; !!task; task = task->parent.lock())
                                sweat_per_task_cumul[task] += sweat;
                        }

                        if (total_sweat > 0.0)
                        {
                            std::sort(RANGE(ordered_prods), [](const Task::Ptr &a, const Task::Ptr &b){return a->fullName() < b->fullName();});

                            set<string> shown;
                            auto stream_line = [&](const string &name, double sweat)
                            {
                                if (shown.count(name))
                                    return;
                                const auto pct = 100.0*sweat/total_sweat;
                                switch (format)
                                {
                                    case ::Format::Text:
                                        os << "      " << name << string(max_name_size-name.size(), ' ') << " (" << sweat << "days, " << std::fixed << pct << std::defaultfloat << "%)" << endl;
                                        break;
                                    case ::Format::Html:
                                        if (!!table)
                                        {
                                            auto tr = table->tag("tr");
                                            tr.tag("td") << "";
                                            tr.tag("td") << name;
                                            tr.tag("td") << sweat << " days";
                                            tr.tag("td") << pct << " %";
                                        }
                                        break;
                                }
                                shown.insert(name);
                            };

                            std::function<void(const Task::Ptr &ptr, int &level, const int level_to_show)> stream_sweat;
                            stream_sweat = [&](const Task::Ptr &ptr, int &level, const int level_to_show)
                            {
                                if (!ptr)
                                    return;
                                const auto &task = *ptr;
                                const auto sweat = sweat_per_task_cumul[ptr];

                                if (level_to_show >= 0)
                                {
                                    //We have to start counting from the root
                                    stream_sweat(task.parent.lock(), level, level_to_show);
                                    if (level_to_show == level)
                                        stream_line(task.fullName(), sweat);
                                    //Increment after recursive call
                                    ++level;
                                }
                                else
                                {
                                    //We have to start counting from the leaf
                                    //Decrement before recursive call
                                    --level;
                                    if (level_to_show == level)
                                        stream_line(task.fullName(), sweat);
                                    stream_sweat(task.parent.lock(), level, level_to_show);
                                }
                            };

                            switch (format)
                            {
                                case ::Format::Text:
                                    os << " * " << *days.begin() << " - " << *days.rbegin() << endl;
                                    os << "    * Total occupation: " << total_sweat << "days" << endl;
                                    break;
                                case ::Format::Html:
                                    if (!table)
                                    {
                                        table.reset(new Tag(body->tag("table")));
                                        table->attr("border", "1");
                                        auto tr = table->tag("tr");
                                        tr.tag("th") << "Level";
                                        tr.tag("th") << "Task";
                                        tr.tag("th") << "Days";
                                        tr.tag("th") << "Pct";
                                    }
                                    {
                                        auto tr = table->tag("tr");
                                        tr.tag("th") << *days.begin() << " - " << *days.rbegin();
                                        tr.tag("th") << "";
                                        tr.tag("td") << total_sweat << " days";
                                        tr.tag("th") << "";
                                    }
                                    break;
                            }
                            auto stream_level = [&](const string &level)
                            {
                                switch (format)
                                {
                                    case ::Format::Text:
                                        os << "    * " << level << endl;
                                        break;
                                    case ::Format::Html:
                                        {
                                            auto tr = table->tag("tr");
                                            tr.tag("td") << level;
                                            tr.tag("td") << "";
                                            tr.tag("td") << "";
                                            tr.tag("td") << "";
                                        }
                                        break;
                                }
                            };
                            {
                                stream_level("Overview");
                                shown.clear();
                                for (const auto &ptr: ordered_prods)
                                {
                                    int level = 0;
                                    stream_sweat(ptr, level, 1);
                                }
                            }
                            {
                                stream_level("Details");
                                shown.clear();
                                for (const auto &ptr: ordered_prods)
                                {
                                    int level = 0;
                                    stream_sweat(ptr, level, -1);
                                }
                            }
                        }
                    }
                    table.reset();
                }
            }
            break;
    }
    MSS_END();
}
pa::ReturnCode Plan::execute(const Options &options)
{
    MSS_BEGIN(ReturnCode, logns);

    using namespace gubg::planning;
    Planner planner;
    MSS(planner.add_workers(options.workers));
    MSS(planner.run());
    planner.root->stream(cout);
    MSS(stream(cout, planner, view_, ::Format::Text));
    if (!options.output.name().empty())
    {
        {
            gubg::file::File fn(options.output.name());
            fn.setExtension("txt");
            ofstream fo(fn.name());
            MSS(stream(fo, planner, view_, ::Format::Text));
        }
        {
            gubg::file::File fn(options.output.name());
            fn.setExtension("html");
            ofstream fo(fn.name());
            MSS(stream(fo, planner, view_, ::Format::Html));
        }
        if (view_ == Plan::Products)
        {
            gubg::file::File fn(options.output.name());
            fn.setExtension("timeline");
            ofstream fo(fn.name());
            MSS(stream(fo, planner, view_, ::Format::Timeline));
        }
    }

    MSS_END();
}
