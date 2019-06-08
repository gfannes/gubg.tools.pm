#ifndef HEADER_pit_Resource_hpp_ALREADY_INCLUDED
#define HEADER_pit_Resource_hpp_ALREADY_INCLUDED

#include <string>
#include <set>
#include <list>
#include <ostream>

namespace pit { 

    class Skill
    {
    public:
        std::string name;
        double efficiency{};

        bool operator<(const Skill &rhs) const {return name < rhs.name;}

        void stream(std::ostream &os) const
        {
            os << "  [skill](name:" << name << ")(efficiency:" << efficiency << ")";
        }
    private:
    };

    class Resource
    {
    public:
        std::string name;
        double occupancy{};
        std::set<Skill> skills;

        void stream(std::ostream &os) const
        {
            os << "[resource](name:" << name << ")(occupancy:" << occupancy << "){\n";
            for (const auto &skill: skills)
            {
                os << "  "; skill.stream(os); os << std::endl;
            }
            os << "}\n";
        }
    private:
    };
    inline std::ostream &operator<<(std::ostream &os, const Resource &resource)
    {
        resource.stream(os);
        return os;
    }

    using Resources = std::list<Resource>;

} 

#endif
