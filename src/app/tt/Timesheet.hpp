#ifndef HEADER_tt_Timesheet_hpp_ALREADY_INCLUDED
#define HEADER_tt_Timesheet_hpp_ALREADY_INCLUDED

#include "tt/ReturnCode.hpp"
#include "gubg/parse/naft/Parser.hpp"
#include "gubg/planning/Day.hpp"
#include "gubg/time/DayTime.hpp"
#include <string>
#include <map>
#include <array>
#include <vector>
#include <memory>
#include <chrono>
#include <ostream>

namespace tt { 

    using Day = gubg::planning::Day;
    using DayTime = gubg::time::DayTime;
    using Duration = std::chrono::seconds;

    class Timesheet: public gubg::parse::naft::Parser_crtp<Timesheet>
    {
        public:
            ReturnCode filter(unsigned int year, unsigned int month, unsigned int day = 1);

            ReturnCode parse(const std::string &filename);

            bool naft_text(const Text &){return true;}
            bool naft_node_open(const Tag &);
            bool naft_attr(const Key &, const Value &);
            bool naft_attr_done();
            bool naft_node_close();

            Day day() const;

            void stream(std::ostream &) const;

        private:
            using Name = std::string;
            using Attributes = std::map<std::string, std::string>;
            Name name_;
            Attributes attrs_;

            int level_ = -1;
            using YMD = std::array<unsigned int, 3>;
            YMD ymd_;
            YMD::iterator ymd_it_ = ymd_.begin();
            std::vector<std::string> story_stack_;

            struct Info
            {
                std::unique_ptr<DayTime> start;
                std::unique_ptr<DayTime> stop;
                std::map<std::string, std::map<std::string, Duration>> duration_per_task_per_story;
                Duration pause{0};
                void stream(std::ostream &) const;
                void update();
            };
            using InfoPerDay = std::map<Day, Info>;
            InfoPerDay info_per_day_;

            std::unique_ptr<Day> filter_from_day_;

            const DayTime pause_begin_{12,0,0};
            const DayTime pause_end_{12,30,0};
            const Duration minimal_pause_() const {return pause_end_ - pause_begin_;}
    };
    inline std::ostream &operator<<(std::ostream &os, const Timesheet &ts)
    {
        ts.stream(os);
        return os;
    }
} 

#endif
