#include "pa/Plan.hpp"
#include "pa/Planner.hpp"
#include <string>
#include <fstream>
#include <map>
#include <set>
using namespace pa;
using namespace std;

namespace  { 
    const char *logns = "pa::Plan";
} 

    template <typename Task>
string taskDescription(const Task &task)
{
    ostringstream oss;
    oss << task.fullName() << "(" << task.cumulSweat << ")";
    return oss.str();
}
void stream(std::ostream &os, Planner &planner, Plan::Level level, gubg::planning::Format format)
{
    using namespace gubg::planning;
    switch (level)
    {
        case Plan::Overview:
            switch (format)
            {
                case Format::Text:
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
                            os << "deadline: " << *p.first << std::endl;
                        }
                    }
                    break;
                case Format::Html:
                    os << "<html><body>Not implemented yet</body></html>";
                    break;
            }
            break;
        case Plan::Products:
            switch (format)
            {
                case Format::Text:
                    {
                        os << "# Product delivery overview on " << today() << std::endl << std::endl;
                        auto leafTasks = gubg::tree::dfs::leafs(*planner.root);
                        using StopPerProduct = std::map<std::string, Day>;
                        StopPerProduct stop_per_product;
                        for (auto leaf: leafTasks)
                        {
                            auto &task = *leaf;
                            const unsigned int product_level = 4;
                            const auto basename = task.baseName(product_level);
                            auto &stop = stop_per_product[basename];
                            if (stop < task.stop)
                                stop = task.stop;
                        }
                        using ProductsPerStop = std::multimap<Day, std::string>;
                        ProductsPerStop products_per_stop;
                        size_t max_product_size = 0;
                        for (const auto &p: stop_per_product)
                        {
                            const auto &product = p.first;
                            const auto &stop = p.second;
                            products_per_stop.emplace(stop, product);
                            max_product_size = std::max(product.size(), max_product_size);
                        }
                        for (const auto &p: products_per_stop)
                        {
                            const auto &stop = p.first;
                            const auto &product = p.second;
                            assert(product.size() <= max_product_size);
                            os << product << std::string(max_product_size-product.size(), ' ') << " => ETA: " << stop << std::endl;
                        }
                    }
                    break;
                case Format::Html:
                    os << "<html><body>Not implemented yet</body></html>";
                    break;
            }
            break;
        case Plan::Details:
            planner.planning.stream(os, format);
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
    stream(cout, planner, level_, Format::Text);
    if (!options.output.name().empty())
    {
        {
            gubg::file::File fn(options.output.name());
            fn.setExtension("txt");
            ofstream fo(fn.name());
            stream(fo, planner, level_, Format::Text);
        }
        {
            gubg::file::File fn(options.output.name());
            fn.setExtension("html");
            ofstream fo(fn.name());
            stream(fo, planner, level_, Format::Html);
        }
    }

    MSS_END();
}
