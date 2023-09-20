#ifndef HEADER_pit_Day_hpp_ALREADY_INCLUDED
#define HEADER_pit_Day_hpp_ALREADY_INCLUDED

#include <gubg/planning/Day.hpp>
#include <gubg/Range.hpp>
#include <cassert>
#include <optional>

namespace pit { 

    class Day
    {
    public:
        using Base = gubg::planning::Day;
        using Days = std::vector<Day>;

        Base day;
        double occupancy = 0.0;

        Day() {}
        Day(const gubg::planning::Day &d): day(d) { }

        static Day today() 
        {
            Day res;
            res.day = gubg::planning::Day::today();
            return res;
        }
        static Days work_days(unsigned int nr)
        {
            const auto bases = gubg::planning::work_days(nr);
            assert(bases.size() == nr);
            Days days(bases.size());
            std::copy(RANGE(bases), days.begin());
            return days;
        }

        void stream(std::ostream &os) const
        {
            os << day << '%' << occupancy;
        }
        Day &operator++()
        {
            ++day;
            occupancy = 0.0;
            return *this;
        }
        bool operator<(const Day &rhs) const { return std::make_pair(day, occupancy) < std::make_pair(rhs.day, rhs.occupancy); }
        bool operator>(const Day &rhs) const { return rhs.operator<(*this); }
        bool operator>=(const Day &rhs) const { return !operator<(rhs); }
        bool operator<=(const Day &rhs) const { return !operator>(rhs); }
    };
    using Days = Day::Days;
    using Day_opt = std::optional<Day>;

    inline std::ostream &operator<<(std::ostream &os, const Day &day)
    {
        day.stream(os);
        return os;
    }

    inline Day max(const Day &a, const Day_opt &b)
    {
        Day res = a;
        if (b && *b > res)
            res = *b;
        return res;
    }
    inline Day_opt max(const Day_opt &a, const Day_opt &b)
    {
        Day_opt res = a;
        if (!res)
            res = b;
        else if (b && *b > *res)
            res = b;
        return res;
    }

} 

#endif
