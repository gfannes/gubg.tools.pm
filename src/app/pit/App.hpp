#ifndef HEADER_pit_App_hpp_ALREADY_INCLUDED
#define HEADER_pit_App_hpp_ALREADY_INCLUDED

#include <pit/Options.hpp>
#include <pit/Config.hpp>
#include <pit/Model.hpp>
#include <pit/Reporter.hpp>
#include <pit/ResourceMgr.hpp>
#include <gubg/mss.hpp>
#include <cstdlib>
#include <set>
#include <iomanip>

namespace pit { 
    class App
    {
    public:
        bool process(int argc, const char **argv)
        {
            MSS_BEGIN(bool);

            MSS(options_.parse(argc, argv), std::cout << "Error: could not parse cli args" << std::endl);

            {
                auto config_path = config_path_();
                config_.install(config_path);
                MSS(config_.load(config_path), std::cout << "Error: could not load config" << std::endl);
            }

            if (options_.verbose)
                std::cout << options_;
            if (options_.help)
                options_.stream_help(std::cout);

            MSS(model_.load(options_.input_files));
            if (options_.verbose && false)
                std::cout << model_.str();

            if (options_.mode == Mode::Plan)
            {
                MSS(resource_mgr_.load(options_.resources_fn));
                auto lambda = [&](Model::Node &node, bool oc){

                    if (oc)
                        return true;

                    auto todo_minutes = node.todo.value_or(gubg::Army{}).as_minutes();
                    if (todo_minutes == 0)
                        return true;

                    const auto &required_skill = node.required_skill;
                    std::cout << "Planning \"" << node.tag << "\" for skill \"" << required_skill << "\" => " << todo_minutes << "min" << std::endl;

                    auto start_day = Day::today();
                    auto find_start_day = [&](const auto &n){
                        if (n.last >= start_day)
                            start_day = n.last;
                        return true;
                    };
                    node.each_child(find_start_day);

                    for (auto day = start_day; todo_minutes > 0; ++day)
                    {
                        if (!resource_mgr_.is_in_range(day))
                        {
                            std::cout << "Error: could not plan task \"" << node.tag << "\" requiring skill \"" << required_skill << "\", out of days" << std::endl;
                            return false;
                        }

                        std::cout << "  " << day << " " << todo_minutes << "min left" << std::endl;
                        Resources *resources = resource_mgr_.get(day);
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
                                std::cout << "  Assigning to " << resource.name << std::endl;
                                node.worker = resource.name;
                                node.first = day;
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
                        const auto available = 1.0-resource.occupancy;
                        std::cout << "  " << required << " " << available << std::endl;
                        if (required <= available)
                        {
                            //We can finish the task today
                            resource.occupancy += required;
                            todo_minutes = 0;
                            node.last = day;
                        }
                        else
                        {
                            //We cannot finish the task today
                            resource.occupancy = 1.0;
                            int done_minutes = efficiency*minutes_per_day*available;
                            if (done_minutes == 0)
                                //Make sure we do not get stuck in the float-int conversions: keep making progress
                                done_minutes = 1;
                            todo_minutes -= done_minutes;
                        }
                    }

                    return true;
                };
                MSS(model_.traverse(model_.root(), lambda, true));
            }

            if (options_.mode == Mode::Report)
            {
                Reporter reporter;
                MSS(reporter.process(options_));
                MSS(reporter.process(model_));
            }

            MSS_END();
        }

    private:
        std::filesystem::path config_path_() const
        {
            std::filesystem::path res("pit_config.naft");
            auto home = std::getenv("HOME");
            if (home)
            {
                res = home;
                res /= ".config";
                res /= "pit";
                res /= "config.naft";
            }
            return res;
        }

        Options options_;
        Config config_;
        Model model_;
        ResourceMgr resource_mgr_;
    };
} 

#endif
