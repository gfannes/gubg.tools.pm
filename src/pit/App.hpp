#ifndef HEADER_pit_App_hpp_ALREADY_INCLUDED
#define HEADER_pit_App_hpp_ALREADY_INCLUDED

#include <pit/Options.hpp>
#include <pit/Config.hpp>
#include <pit/Model.hpp>
#include <pit/ResourceMgr.hpp>
#include <pit/report.hpp>
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

            configure_log(options_);
            log(Verbose) << options_;

            if (options_.help)
                options_.stream_help(std::cout);

            MSS(model_.load(options_.input_files));
            log(Debug) << model_.str();

            if (options_.plan)
            {
                if (options_.resources_fn)
                    MSS(resource_mgr_.load(*options_.resources_fn, options_.nr_work_days));
                else
                    MSS(resource_mgr_.single_worker(options_.nr_work_days));
                MSS(model_.plan(resource_mgr_));
            }

            MSS(model_.aggregate());

            MSS(report(model_, options_));

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
