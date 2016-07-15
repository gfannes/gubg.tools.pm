#include "tt/Timesheet.hpp"
#include "gubg/file/Filesystem.hpp"
#include "gubg/Strange.hpp"
#include "gubg/mss.hpp"
using namespace std;
using namespace gubg;

namespace  { 
    const char *ns = "Timesheet";
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
            for (const auto &attr: attrs)
            {
                const auto &key = attr.first;
                const auto &value = attr.second;
                if (false) {}
                else if (key == "b")
                {
                    if (!info.start)
                    {
                        DayTime dt;
                        MSS(DayTime::from_armin(dt, value));
                        info.start.reset(new DayTime(dt));
                        info.update();
                    }
                }
                else if (key == "e")
                {
                    MSS(!!info.start);
                    MSS(!!info.stop);
                    DayTime dt;
                    MSS(DayTime::from_armin(dt, value));
                    if (dt > *info.stop)
                    {
                        *info.stop = dt;
                        info.update();
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
            const DayTime ten_am(10,0,0);
            const DayTime two_pm(14,0,0);
            if (ten_am <= *start && *start <= two_pm)
                pause += chrono::minutes(30);
        }
    }
} 
