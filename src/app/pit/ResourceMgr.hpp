#ifndef HEADER_pit_ResourceMgr_hpp_ALREADY_INCLUDED
#define HEADER_pit_ResourceMgr_hpp_ALREADY_INCLUDED

#include <pit/Types.hpp>
#include <pit/Resource.hpp>
#include <gubg/file/System.hpp>
#include <gubg/naft/Range.hpp>
#include <gubg/mss.hpp>
#include <gubg/map.hpp>

namespace pit { 

    class ResourceMgr
    {
    public:
        bool load(const std::string &filename)
        {
            MSS_BEGIN(bool);

            std::string content;
            MSS(gubg::file::read(content, gubg::file::Name{filename}), std::cout << "Error: could not read from \"" << filename << "\"\n");

            for (gubg::naft::Range r0{content}; !r0.empty(); )
            {
                pit::Resource resource;
                pit::Day from;
                MSS(r0.pop_tag(resource.name));
                const auto a0 = r0.pop_attrs();
                gubg::naft::Range r1;
                MSS(r0.pop_block(r1));
                for (; r1.pop_tag("skills"); )
                {
                    const auto a1 = r1.pop_attrs();
                    from = gubg::get_or(Day::today(), a1, "from");
                    gubg::naft::Range r2;
                    MSS(r1.pop_block(r2));
                    for (; !r2.empty();)
                    {
                        pit::Skill skill;
                        MSS(r2.pop_tag(skill.name));
                        const auto a2 = r2.pop_attrs();
                        skill.efficiency = gubg::get_or(1.0, a2, "efficiency");
                        resource.skills.insert(skill);
                    }
                }
                std::cout << from << ": " << resource << std::endl;
            }

            MSS_END();
        }

        bool get(Resources &resources, const Day &day) const
        {
            MSS_BEGIN(bool);
            MSS_END();
        }

    private:
        std::map<Day, Resources> day__resources_;
    };

} 

#endif
