#ifndef HEADER_pit_Config_hpp_ALREADY_INCLUDED
#define HEADER_pit_Config_hpp_ALREADY_INCLUDED

#include "gubg/mss.hpp"
#include "gubg/std/filesystem.hpp"
#include "gubg/naft/Document.hpp"
#include "gubg/naft/Range.hpp"
#include "gubg/file/system.hpp"
#include <fstream>
#include <string>

namespace pit { 
    class Config
    {
    public:
        void install(const std::filesystem::path &path) const
        {
            if (std::filesystem::exists(path))
                return;
            std::filesystem::create_directories(path.parent_path());
            std::ofstream fo(path);
            gubg::naft::Document doc(fo);
            auto config = doc.node("config");
            {
                auto def = config.node("default");
            }
        }
        bool load(const std::filesystem::path &path)
        {
            MSS_BEGIN(bool);
            std::string content;
            MSS(gubg::file::read(content, path));
            gubg::naft::Range range(content);
            MSS(range.pop_tag("config"), std::cout << "Error: could not find \"config\" tag" << std::endl);
            MSS_END();
        }

    private:
        std::string default_;
    };
} 

#endif
