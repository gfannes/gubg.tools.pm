#ifndef HEADER_pit_Resource_hpp_ALREADY_INCLUDED
#define HEADER_pit_Resource_hpp_ALREADY_INCLUDED

#include <string>
#include <set>
#include <list>

namespace pit { 

    class Skill
    {
    public:
        std::string name;
        double efficiency{};
        bool operator<(const Skill &rhs) const {return name < rhs.name;}
    private:
    };

    class Resource
    {
    public:
        std::string name;
        double occupancy{};
        std::set<Skill> skills;
    private:
    };

    using Resources = std::list<Resource>;

} 

#endif
