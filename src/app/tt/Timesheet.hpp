#ifndef HEADER_tt_Timesheet_hpp_ALREADY_INCLUDED
#define HEADER_tt_Timesheet_hpp_ALREADY_INCLUDED

#include "tt/ReturnCode.hpp"
#include "gubg/parse/tree/Parser.hpp"
#include "gubg/planning/Day.hpp"
#include "gubg/time/DayTime.hpp"
#include <string>
#include <map>
#include <array>
#include <memory>
#include <chrono>
#include <ostream>

namespace tt { 

    using Day = gubg::planning::Day;
    using DayTime = gubg::time::DayTime;

    class Timesheet: public gubg::parse::tree::Parser_crtp<Timesheet>
    {
        public:
            ReturnCode parse(const std::string &filename);

            using Name = gubg::parse::tree::Name;
            using Attributes = gubg::parse::tree::Attributes;
            bool parser_open(const Name &, const Attributes &);
            bool parser_close();

            Day day() const;

            void stream(std::ostream &) const;

        private:
            int level_ = -1;
            using YMD = std::array<unsigned int, 3>;
            YMD ymd_;
            YMD::iterator ymd_it_ = ymd_.begin();

            struct Info
            {
                std::unique_ptr<DayTime> start;
                std::unique_ptr<DayTime> stop;
                std::chrono::seconds pause{0};
                void stream(std::ostream &) const;
                void update();
            };
            using InfoPerDay = std::map<Day, Info>;
            InfoPerDay info_per_day_;
    };
    inline std::ostream &operator<<(std::ostream &os, const Timesheet &ts)
    {
        ts.stream(os);
        return os;
    }
} 

#endif
