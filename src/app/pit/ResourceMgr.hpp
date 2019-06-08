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

            std::map<std::string, std::map<Day, Resource>> name__day__resource;
            for (gubg::naft::Range r0{content}; !r0.empty(); )
            {
                std::string name;
                pit::Day from;
                MSS(r0.pop_tag(name));
                const auto a0 = r0.pop_attrs();
                gubg::naft::Range r1;
                MSS(r0.pop_block(r1));
                for (; r1.pop_tag("skills"); )
                {
                    pit::Resource resource{name};
                    const auto a1 = r1.pop_attrs();
                    from = gubg::value_or(Day::today(), a1, "from");
                    gubg::naft::Range r2;
                    MSS(r1.pop_block(r2));
                    for (; !r2.empty();)
                    {
                        std::string skill;
                        MSS(r2.pop_tag(skill));
                        const auto a2 = r2.pop_attrs();
                        resource.skill__efficiency[skill] = gubg::value_or(1.0, a2, "efficiency");
                    }
                    name__day__resource[name][from] = resource;
                }
            }

            const auto days = gubg::planning::work_days(20);
            for (const auto &p: name__day__resource)
            {
                const auto &name = p.first;
                const auto &day__resource = p.second;
                for (const auto &day: days)
                {
                    auto it = day__resource.upper_bound(day);
                    day__resources_[day].push_back(
                            it == day__resource.begin() ?
                            Resource{name} :
                            (--it, it->second)
                            );
                }
            }

            MSS_END();
        }

        void stream(std::ostream &os) const
        {
            for (const auto &p: day__resources_)
            {
                const auto &day = p.first;
                os << std::endl << day << std::endl;
                const auto &resources = p.second;
                for (const auto &resource: resources)
                    os << resource;
            }
        }

        Resources * get(const Day &day)
        {
            auto it = day__resources_.find(day);
            if (it == day__resources_.end())
                return nullptr;
            return &it->second;
        }

        bool is_in_range(const Day &day) const
        {
            auto it = day__resources_.lower_bound(day);
            return it != day__resources_.end();
        }

    private:
        std::map<Day, Resources> day__resources_;
    };

} 

#endif
