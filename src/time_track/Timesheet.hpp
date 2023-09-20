#ifndef HEADER_time_track_Timesheet_hpp_ALREADY_INCLUDED
#define HEADER_time_track_Timesheet_hpp_ALREADY_INCLUDED

#include <time_track/Options.hpp>
#include <time_track/ReturnCode.hpp>

#include <gubg/parse/naft/Parser.hpp>
#include <gubg/planning/Day.hpp>
#include <gubg/time/DayTime.hpp>
#include <gubg/Logger.hpp>

#include <string>
#include <map>
#include <array>
#include <vector>
#include <memory>
#include <chrono>
#include <ostream>
#include <optional>

namespace time_track { 

    using Day = gubg::planning::Day;
    using DayTime = gubg::time::DayTime;
    using Duration = std::chrono::seconds;
    struct WorkDeepFocus
    {
        time_track::Duration work{};
        time_track::Duration deep{};
        time_track::Duration focus{};

        WorkDeepFocus &operator+=(const WorkDeepFocus &rhs);
    };

    class Timesheet: public gubg::parse::naft::Parser_crtp<Timesheet>
    {
        public:
            const Options &options;
            gubg::Logger &log;
            Timesheet(const Options &options, gubg::Logger &log): options(options), log(log) {}

            ReturnCode filter_from(unsigned int year, unsigned int month, unsigned int day = 1);
            ReturnCode filter_until(unsigned int year, unsigned int month, unsigned int day = 1);

            ReturnCode parse(const std::string &filename);

            //naft::Parser_crtp callbacks
            bool naft_text(const Text &){return true;}
            bool naft_node_open(const Tag &);
            bool naft_attr(const Key &, const Value &);
            bool naft_attr_done();
            bool naft_node_close();

            Day day() const;

            void stream(std::ostream &) const;
            void stream_totals(std::ostream &os) const;

        private:
            using Name = std::string;
            using Attributes = std::map<std::string, std::string>;
            Name name_;
            Attributes attrs_;

            int level_ = -1;
            using YMD = std::array<unsigned int, 3>;
            YMD ymd_;
            YMD::iterator ymd_it_ = ymd_.begin();

            struct StoryInfo
            {
                std::string story;
                bool deep = false;
                bool focus = false;
            };
            std::vector<StoryInfo> storyinfo_stack_;

            struct DayInfo
            {
                std::unique_ptr<DayTime> start;
                std::unique_ptr<DayTime> stop;
                std::map<std::string, std::map<std::string, WorkDeepFocus>> story__task__wdf;
                Duration pause{0};
                std::optional<double> power;
                void stream(std::ostream &) const;
                void update();
            };
            using Day__DayInfo = std::map<Day, DayInfo>;
            Day__DayInfo day__dayinfo_;

            std::unique_ptr<Day> filter_from_day_;
            std::unique_ptr<Day> filter_until_day_;

            const DayTime pause_begin_{12,40,0};
            const DayTime pause_end_{13,25,0};
            const Duration minimal_pause_() const {return pause_end_ - pause_begin_;}
    };
    inline std::ostream &operator<<(std::ostream &os, const Timesheet &ts)
    {
        ts.stream(os);
        return os;
    }
} 

#endif
