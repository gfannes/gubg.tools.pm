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

            MSS(resource_mgr_.load(options_.resources_fn));
            MSS(model_.load(options_.input_files));

            if (options_.verbose && false)
                std::cout << model_.str();

            Reporter reporter;
            MSS(reporter.process(options_));
            MSS(reporter.process(model_));

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
