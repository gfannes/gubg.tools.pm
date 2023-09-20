#ifndef HEADER_pit_Resource_hpp_ALREADY_INCLUDED
#define HEADER_pit_Resource_hpp_ALREADY_INCLUDED

#include <gubg/map.hpp>
#include <string>
#include <set>
#include <list>
#include <ostream>

namespace pit { 

    class Resource
    {
    public:
        std::string name;
        double occupancy{};
        std::map<std::string, double> skill__efficiency;

        Resource() {}
        Resource(const std::string &name): name(name) {}

        double efficiency(const std::string &skill) const
        {
            return gubg::value_or(0.0, skill__efficiency, skill);
        }

        void stream(std::ostream &os) const
        {
            os << "[resource](name:" << name << ")(occupancy:" << occupancy << "){\n";
            for (const auto &p: skill__efficiency)
                os << "  [skill](name:" << p.first << ")(efficiency:" << p.second << ")" << std::endl;
            os << "}\n";
        }
    private:
    };
    inline std::ostream &operator<<(std::ostream &os, const Resource &resource)
    {
        resource.stream(os);
        return os;
    }

    class Resources: public std::list<Resource>
    {
    public:
        //Returns all resources that have the specified skill, ordered by efficiency for that skill
        Resource * get_most_efficient_for(const std::string &skill)
        {
            double efficiency = -1.0;
            Resource *ptr = nullptr;

            for (auto &r: *this)
            {
                if (r.occupancy >= 1.0)
                    continue;
                const auto eff = r.efficiency(skill);
                if (eff <= 0.0)
                    continue;
                if (eff > efficiency)
                {
                    efficiency = eff;
                    ptr = &r;
                }
            }

            return ptr;
        }
        Resource * get_by_name(const std::string &name)
        {
            for (auto &r: *this)
                if (r.name == name)
                    return &r;
            return nullptr;
        }

    private:
    };

} 

#endif
