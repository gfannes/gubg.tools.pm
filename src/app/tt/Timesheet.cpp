#include "tt/Timesheet.hpp"
#include "gubg/file/Filesystem.hpp"
#include "gubg/Strange.hpp"
#include "gubg/mss.hpp"
using namespace std;
using namespace gubg;

namespace  { 
    const char *ns = "Timesheet";
    std::string as_hours(tt::Duration duration)
    {
        std::ostringstream oss;
        oss << std::fixed << double(duration.count())/3600.0;
        return oss.str();
    }
} 

namespace tt { 
    ReturnCode Timesheet::parse(const string &filename)
    {
        MSS_BEGIN(ReturnCode);
        string content;
        MSS(file::read(content, filename));
        MSS(process(content));
        MSS_END();
    }

    bool Timesheet::parser_open(const Name &name, const Attributes &attrs)
    {
        MSS_BEGIN(bool, ns);
        L(C(name));
        
        ++level_;

        //We start with the story from the previous level, if any
        story_stack_.push_back(!story_stack_.empty() ? story_stack_.back() : "");
        MSS(story_stack_.size() == level_+1);

        if (ymd_it_ != ymd_.end())
        {
            L("Day is not known yet");
            if ((ymd_it_ - ymd_.begin()) == level_)
            {
                Strange strange(name);
                if (!strange.pop_decimal(*ymd_it_))
                {
                    L("This is not a number, I will skip it");
                }
                else
                {
                    ++ymd_it_;
                }
            }
        }

        if (ymd_it_ == ymd_.end())
        {
            const auto d = day();
            L("Day is known: " << d);

            auto &info = info_per_day_[d];

            MSS(story_stack_.size() == level_+1);
            auto &story = story_stack_[level_];
            const auto &task = name;

            //We check for the begin time before anything else
            {
                for (const auto &attr: attrs)
                {
                    const auto &key = attr.first;
                    const auto &value = attr.second;
                    if (false) {}
                    else if (key == "b")
                    {
                        if (!info.start)
                        {
                            MSS(info.duration_per_task_per_story.empty(), L("There are durations present: you cannot start using a start time anymore"));
                            DayTime dt;
                            MSS(DayTime::from_armin(dt, value));
                            info.start.reset(new DayTime(dt));
                            info.update();
                        }
                    }
                    else if (key == "s")
                    {
                        story = value;
                    }
                }
            }

            if (!info.start)
            {
                //Using durations directly
                for (const auto &attr: attrs)
                {
                    const auto &key = attr.first;
                    const auto &value = attr.second;
                    if (false) {}
                    else if (key == "b") { MSS(false); }
                    else if (key == "e") { MSS(false); }
                    else if (key == "d")
                    {
                        DayTime dt;
                        MSS(DayTime::from_armin(dt, value));
                        info.duration_per_task_per_story[story][task] += dt.duration();
                    }
                }
            }
            else
            {
                //Using a start time
                MSS(!!info.stop);

                const auto previous_stop = *info.stop;
                auto &stop = *info.stop;

                for (const auto &attr: attrs)
                {
                    const auto &key = attr.first;
                    const auto &value = attr.second;
                    if (false) {}
                    else if (key == "b")
                    {
                        //Already handled
                    }
                    else if (key == "e")
                    {
                        DayTime dt;
                        MSS(DayTime::from_armin(dt, value));
                        if (dt > stop)
                        {
                            stop = dt;
                            info.update();
                        }
                    }
                    else if (key == "d")
                    {
                        DayTime dt;
                        MSS(DayTime::from_armin(dt, value));
                        stop += dt.duration();
                        if (previous_stop <= pause_begin_)
                        {
                            if (stop <= pause_begin_)
                            {
                                //Both before noon: no adjustment to make
                            }
                            else if (pause_end_ <= stop)
                            {
                                L("Converting this to a full pause skip");
                                //A duration crossing pause_begin_ will take the pause fully
                                //We add this pause time, which will be reversed hereunder during pause cross detection
                                stop += (pause_end_ - pause_begin_);
                            }
                        }
                    }
                }

                auto &current_stop = *info.stop;

                if (previous_stop != current_stop)
                {
                    L(C(story)C(previous_stop)C(current_stop));

                    if (previous_stop <= pause_begin_)
                    {
                        if (current_stop <= pause_begin_)
                        {
                            //Before pause: no problem
                            info.duration_per_task_per_story[story][task] += current_stop - previous_stop;
                        }
                        else if (current_stop <= pause_end_)
                        {
                            L("Ends in the middle of the pause: we take a break now");
                            info.duration_per_task_per_story[story][task] += current_stop - previous_stop;
                            current_stop += (pause_end_ - pause_begin_);
                            assert(pause_end_ <= current_stop);
                        }
                        else
                        {
                            L("Skips the pause: we split this work and assume we paused in-between");
                            info.duration_per_task_per_story[story][task] += pause_begin_ - previous_stop;
                            info.duration_per_task_per_story[story][task] += current_stop - pause_end_;
                        }
                    }
                    else
                    {
                        //We assume pause is already taken
                        info.duration_per_task_per_story[story][task] += current_stop - previous_stop;
                    }
                }
            }
        }

        MSS_END();
    }
    bool Timesheet::parser_close()
    {
        MSS_BEGIN(bool, ns);
        if ((ymd_it_-ymd_.begin()) == (level_+1))
            --ymd_it_;
        --level_;
        story_stack_.pop_back();
        MSS_END();
    }

    Day Timesheet::day() const
    {
        Day d;
        if (ymd_it_ == ymd_.end())
            d = Day(ymd_[0], ymd_[1], ymd_[2]);
        return d;
    }

    void Timesheet::stream(std::ostream &os) const
    {
        for (const auto &di: info_per_day_)
        {
            const auto &day = di.first;
            const auto &info = di.second;
            os << day << ": ";
            info.stream(os);
            Duration total_worked(0);
            for (const auto &p: info.duration_per_task_per_story)
            {
                const auto &story = p.first;
                if (story != "pause")
                {
                    Duration sub_total_worked(0);
                    for (const auto &pp: p.second)
                    {
                        const auto &task = pp.first;
                        const auto &duration = pp.second;
                        sub_total_worked += duration;
                        os << endl << "\t * " << story << "\t" << as_hours(duration) << "\t" << task;
                        if (story.empty())
                            os << "                                 !!! This task has not story yet !!!";
                    }
                    os << endl << "\t       => \t" << as_hours(sub_total_worked);
                    total_worked += sub_total_worked;
                }
            }
            os << endl << "\tTOTAL  =>" << "\t" << as_hours(total_worked);
            os << endl;
        }
    }

    //Timesheet::Info
    void Timesheet::Info::stream(std::ostream &os) const
    {
        if (!!start && !!stop)
            os << "[" << *start << " - " << *stop << "]";
    }
    void Timesheet::Info::update()
    {
        if (!!start)
        {
            if (!stop)
                stop.reset(new DayTime(*start));
        }
    }
} 
