#include "pa/Plan.hpp"
#include "pa/Planner.hpp"
#include "gubg/xml/Builder.hpp"
#include <string>
#include <fstream>
#include <map>
#include <set>
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
void stream(ostream &os, Planner &planner, Plan::View view, Format format)
{
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
                        body.tag("h1") << "Planning overview on " << today();
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
                        if (!start.isValid() || start_ < start)
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
                            os << "# Product delivery overview on " << today() << endl << endl;
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
                            body.tag("h1") << "Product delivery overview on " << today();
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
    }
}
pa::ReturnCode Plan::execute(const Options &options)
{
    MSS_BEGIN(ReturnCode, logns);

    using namespace gubg::planning;
    Planner planner;
    MSS(planner.add_workers(options.workers));
    MSS(planner.run());
    planner.root->stream(cout);
    stream(cout, planner, view_, ::Format::Text);
    if (!options.output.name().empty())
    {
        {
            gubg::file::File fn(options.output.name());
            fn.setExtension("txt");
            ofstream fo(fn.name());
            stream(fo, planner, view_, ::Format::Text);
        }
        {
            gubg::file::File fn(options.output.name());
            fn.setExtension("html");
            ofstream fo(fn.name());
            stream(fo, planner, view_, ::Format::Html);
        }
        if (view_ == Plan::Products)
        {
            gubg::file::File fn(options.output.name());
            fn.setExtension("timeline");
            ofstream fo(fn.name());
            stream(fo, planner, view_, ::Format::Timeline);
        }
    }

    MSS_END();
}
