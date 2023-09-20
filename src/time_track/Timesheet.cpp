#include <time_track/Timesheet.hpp>

#include <gubg/Strange.hpp>
#include <gubg/file/Filesystem.hpp>
#include <gubg/mss.hpp>

#include <cmath>
#include <list>
#include <sstream>

using namespace gubg;

namespace {
    const char *ns = "Timesheet";

    struct AsHours
    {
        const time_track::Duration duration;
        AsHours(time_track::Duration duration)
            : duration(duration) {}
        double count() const { return double(duration.count()) / 3600.0; }
    };
    inline std::ostream &operator<<(std::ostream &os, const AsHours &v)
    {
        return os << std::fixed << std::setprecision(2) << double(v.duration.count()) / 3600.0;
    }

    struct AsPct
    {
        const time_track::Duration part;
        const time_track::Duration total;
        AsPct(time_track::Duration part, time_track::Duration total)
            : part(part), total(total) {}
    };
    inline std::ostream &operator<<(std::ostream &os, const AsPct &v)
    {
        const double part_count = double(v.part.count());
        const double total_count = double(v.total.count());
        if (total_count == 0)
            os << "-";
        else
            os << std::fixed << std::setprecision(1) << std::lround(1000.0 * (part_count / total_count)) / 10.0 << '%';
        return os;
    }

    inline bool yesno(const std::string &str)
    {
        return str.empty() || str == "true" || str == "y" || str == "yes" || str == "1";
    }
} // namespace

namespace time_track {

    // WorkDeepFocus
    WorkDeepFocus &WorkDeepFocus::operator+=(const WorkDeepFocus &rhs)
    {
        work += rhs.work;
        deep += rhs.deep;
        focus += rhs.focus;
        return *this;
    }
    std::ostream &operator<<(std::ostream &os, const WorkDeepFocus &wdf)
    {
        os << AsHours{wdf.work};
        if (wdf.deep != Duration::zero() || wdf.focus != Duration::zero())
            os << " (deep " << AsPct{wdf.deep, wdf.work} << ", focus " << AsPct{wdf.focus, wdf.work} << ", waste " << AsHours{wdf.work - std::max(wdf.deep, wdf.focus)} << ")";
        return os;
    }

    // Timesheet
    ReturnCode Timesheet::filter_from(unsigned int year, unsigned int month, unsigned int day)
    {
        MSS_BEGIN(ReturnCode);
        filter_from_day_.reset(new Day(year, month, day));
        MSS_END();
    }
    ReturnCode Timesheet::filter_until(unsigned int year, unsigned int month, unsigned int day)
    {
        MSS_BEGIN(ReturnCode);
        filter_until_day_.reset(new Day(year, month, day));
        MSS_END();
    }

    ReturnCode Timesheet::parse(const std::string &filename)
    {
        MSS_BEGIN(ReturnCode);
        std::string content;
        MSS(file::read(content, filename), log.error() << "Could not read content from '" << filename << "'" << std::endl);
        MSS(process(content), log.error() << "Failed to parse content from '" << filename << "'" << std::endl);
        MSS_END();
    }

    bool Timesheet::naft_node_open(const Tag &tag)
    {
        MSS_BEGIN(bool);
        name_ = tag;
        attrs_.clear();
        MSS_END();
    }
    bool Timesheet::naft_attr(const Key &key, const Value &value)
    {
        MSS_BEGIN(bool);
        attrs_[key] = value;
        MSS_END();
    }
    bool Timesheet::naft_attr_done()
    {
        MSS_BEGIN(bool, ns);

        L(C(name_) C(level_));

        ++level_;

        // We start with the story from the previous level, if any
        if (storyinfo_stack_.empty())
            storyinfo_stack_.push_back(StoryInfo{});
        else
            storyinfo_stack_.push_back(storyinfo_stack_.back());

        MSS(storyinfo_stack_.size() == level_ + 1);

        if (ymd_it_ != ymd_.end())
        {
            L("Day is not known yet");
            if ((ymd_it_ - ymd_.begin()) == level_)
            {
                Strange strange(name_);
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

            auto &dayinfo = day__dayinfo_[d];

            MSS(storyinfo_stack_.size() == level_ + 1);
            auto &storyinfo = storyinfo_stack_[level_];
            const auto &task = name_;

            // We check for the begin time before anything else
            {
                for (const auto &attr : attrs_)
                {
                    const auto &key = attr.first;
                    const auto &value = attr.second;
                    if (false) {}
                    else if (key == "b")
                    {
                        if (!dayinfo.start)
                        {
                            MSS(dayinfo.story__task__wdf.empty(), log.error() << "There are durations present: you cannot start using a start time anymore" << std::endl);
                            DayTime dt;
                            MSS(DayTime::from_armin(dt, value), log.error() << "Could not interpret DayTime from '" << value << "'" << std::endl);
                            dayinfo.start.reset(new DayTime(dt));
                            dayinfo.update();
                        }
                    }
                    else if (key == "s")
                    {
                        storyinfo.story = value;
                    }
                    else if (key == "deep")
                    {
                        storyinfo.deep = yesno(value);
                    }
                    else if (key == "focus")
                    {
                        storyinfo.focus = yesno(value);
                    }
                    else if (key == "power")
                    {
                        dayinfo.power = std::stod(value);
                    }
                }
            }

            if (!dayinfo.start)
            {
                // Using durations directly
                for (const auto &attr : attrs_)
                {
                    const auto &key = attr.first;
                    const auto &value = attr.second;
                    if (false) {}
                    else if (key == "b") { MSS(false, log.error() << "Unexpected begin found" << std::endl); }
                    else if (key == "e") { MSS(false, log.error() << "Cannot have an end before a begin" << std::endl); }
                    else if (key == "d")
                    {
                        DayTime dt;
                        MSS(DayTime::from_armin(dt, value), log.error() << "Could not interpret DayTime from '" << value << "'" << std::endl);
                        auto &wdf = dayinfo.story__task__wdf[storyinfo.story][task];
                        wdf.work += dt.duration();
                        if (storyinfo.deep) wdf.deep += dt.duration();
                        if (storyinfo.focus) wdf.focus += dt.duration();
                    }
                }
            }
            else
            {
                // Using a start time
                MSS(!!dayinfo.stop);

                const auto previous_stop = *dayinfo.stop;
                auto &stop = *dayinfo.stop;

                for (const auto &attr : attrs_)
                {
                    const auto &key = attr.first;
                    const auto &value = attr.second;
                    if (false) {}
                    else if (key == "b")
                    {
                        // Already handled
                    }
                    else if (key == "e")
                    {
                        DayTime dt;
                        MSS(DayTime::from_armin(dt, value), log.error() << "Could not interpret DayTime from '" << value << "'" << std::endl);
                        if (dt > stop)
                        {
                            stop = dt;
                            dayinfo.update();
                        }
                    }
                    else if (key == "d")
                    {
                        DayTime dt;
                        MSS(DayTime::from_armin(dt, value), log.error() << "Could not interpret DayTime from '" << value << "'" << std::endl);
                        stop += dt.duration();
                        if (previous_stop <= pause_begin_)
                        {
                            if (stop <= pause_begin_)
                            {
                                // Both before noon: no adjustment to make
                            }
                            else if (pause_end_ <= stop)
                            {
                                L("Converting this to a full pause skip");
                                // A duration crossing pause_begin_ will take the pause fully
                                // We add this pause time, which will be reversed hereunder during pause cross detection
                                stop += (pause_end_ - pause_begin_);
                            }
                        }
                    }
                }

                auto &current_stop = *dayinfo.stop;

                if (previous_stop != current_stop)
                {
                    L(C(storyinfo.story) C(previous_stop) C(current_stop));

                    Duration work{};
                    if (previous_stop <= pause_begin_)
                    {
                        if (current_stop <= pause_begin_)
                        {
                            // Before pause: no problem
                            work += current_stop - previous_stop;
                        }
                        else if (current_stop < pause_end_)
                        {
                            L("Ends in the middle of the pause: we take a break now");
                            work += current_stop - previous_stop;
                            dayinfo.pause += minimal_pause_();
                            current_stop += minimal_pause_();
                            assert(pause_end_ <= current_stop);
                        }
                        else
                        {
                            L("Skips the pause: we split this work and assume we paused in-between");
                            work += pause_begin_ - previous_stop;
                            work += current_stop - pause_end_;
                            dayinfo.pause += minimal_pause_();
                        }
                    }
                    else
                    {
                        // We assume pause is already taken
                        work += current_stop - previous_stop;
                    }

                    auto &wdf = dayinfo.story__task__wdf[storyinfo.story][task];
                    wdf.work += work;
                    if (storyinfo.deep) wdf.deep += work;
                    if (storyinfo.focus) wdf.focus += work;
                }
            }
        }

        MSS_END();
    }
    bool Timesheet::naft_node_close()
    {
        MSS_BEGIN(bool, ns);
        if ((ymd_it_ - ymd_.begin()) == (level_ + 1))
            --ymd_it_;
        --level_;
        storyinfo_stack_.pop_back();
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
        const auto today = gubg::planning::Day::today();

        using Story__Day__Details = std::map<std::string, std::map<Day, std::pair<WorkDeepFocus, std::list<std::string>>>>;
        Story__Day__Details story__day__details;

        for (const auto &ddi : day__dayinfo_)
        {
            const auto &day = ddi.first;
            const auto &dayinfo = ddi.second;

            if (filter_from_day_ && day < *filter_from_day_)
                continue;
            if (filter_until_day_ && day >= *filter_until_day_)
                continue;

            if (day == today)
                os << std::endl
                   << "***********************************************************" << std::endl;

            os << day << ": ";
            dayinfo.stream(os);
            WorkDeepFocus total_wdf;
            for (const auto &stw : dayinfo.story__task__wdf)
            {
                const auto &story = stw.first;
                if (story != "pause")
                {
                    WorkDeepFocus subtotal_wdf;
                    for (const auto &tw : stw.second)
                    {
                        const auto &task = tw.first;
                        const auto &wdf = tw.second;
                        {
                            auto &details = story__day__details[story][day];
                            details.first += wdf;
                            details.second.push_back(task);
                        }
                        subtotal_wdf += wdf;
                        os << std::endl
                           << "\t * " << story << "\t" << wdf;
                        os << "\t" << task;
                    }
                    os << std::endl
                       << "\t       => \t" << subtotal_wdf;
                    total_wdf += subtotal_wdf;
                }
            }

            std::string msg;
            {
                const DayTime eight_hours(8, 0, 0);
                if (total_wdf.work < eight_hours.duration())
                {
                    if (!!dayinfo.stop)
                    {
                        auto eight_hour_end = *dayinfo.stop;
                        eight_hour_end += (eight_hours.duration() - total_wdf.work);
                        if (dayinfo.pause < minimal_pause_())
                            eight_hour_end += (minimal_pause_() - dayinfo.pause);
                        std::ostringstream oss;
                        oss << "\tYou have to work until " << eight_hour_end;
                        msg = oss.str();
                    }
                }
            }

            os << std::endl
               << "\tTOTAL  =>"
               << "\t" << total_wdf << msg << std::endl;

            if (day == today)
                os << "***********************************************************" << std::endl
                   << std::endl;
        }

        if (filter_from_day_ || filter_until_day_)
        {
            for (const auto &sdd : story__day__details)
            {
                const auto &story = sdd.first;
                WorkDeepFocus total_wdf;
                for (const auto &dd : sdd.second)
                {
                    const auto &day = dd.first;
                    const auto &wdf = dd.second.first;
                    const auto &tasks = dd.second.second;
                    os << story << ": " << day << ": " << wdf << std::endl;
                    for (const auto &task : tasks)
                        os << task << "; ";
                    os << std::endl;
                    total_wdf += wdf;
                }
                os << "TOTAL: " << total_wdf << std::endl
                   << std::endl;
            }
        }
    }

    void Timesheet::stream_totals(std::ostream &os) const
    {
        auto round_2d = [](double v) { return std::round(100.0 * v) / 100.0; };

        const auto today = gubg::planning::Day::today();
        const double hour_rate = round_2d(options.hour_rate);
        const bool include_details = false;

        using Story__Day__Details = std::map<std::string, std::map<Day, std::pair<WorkDeepFocus, std::list<std::string>>>>;
        Story__Day__Details story__day__details;

        WorkDeepFocus total_wdf;
        std::ostringstream latex, latex_rnd, latex_power, tmp;
        latex << "\\begin{longtable}{@{\\extracolsep{\\fill}\\hspace{\\tabcolsep}} l r r }" << std::endl;
        latex << "\\hline" << std::endl;
        latex << "{\\bf Omschrijving} & \\multicolumn{1}{c}{\\bf Aantal} & \\multicolumn{1}{c}{\\bf Eenheidsprijs (\\euro)} \\\\*" << std::endl;
        latex << "\\hline\\hline" << std::endl;
        latex << "\\endhead" << std::endl;

        latex_rnd << "\\textbf{Research \\& Development} \\\\" << std::endl;

        latex_power << "\\textbf{Stroomverbruik buildservers} \\\\" << std::endl;
        const double price_per_kWh = 0.75;

        double total_hours = 0.0, total_rnd_cost = 0.0, total_power_cost = 0.0;

        std::list<std::vector<std::string>> table;
        table.push_back({"Day", "Work-day", "Focus-day", "Pct-day", "Work-cumul", "Focus-cumul", "Pct-cumul"});
        std::optional<double> prev_power;
        std::vector<double> powers;
        bool is_first = true;
        unsigned int day_count = 0;
        for (const auto &ddi : day__dayinfo_)
        {
            const auto &day = ddi.first;
            const auto &dayinfo = ddi.second;

            if (dayinfo.power)
                prev_power = dayinfo.power;

            if (filter_from_day_ && day < *filter_from_day_)
                continue;
            if (filter_until_day_ && day >= *filter_until_day_)
                continue;

            std::optional<double> power_opt = dayinfo.power;
            if (is_first)
            {
                if (!power_opt)
                    power_opt = prev_power;
                is_first = false;
            }
            if (power_opt)
            {
                const auto power = round_2d(*power_opt);
                powers.push_back(power);
                latex_power << std::fixed << std::setprecision(2);
                latex_power << day << " & " << power << " kWh & " << price_per_kWh << " \\\\" << std::endl;
            }

            WorkDeepFocus total_wdf_day;
            for (const auto &stw : dayinfo.story__task__wdf)
            {
                const auto &story = stw.first;
                if (story != "pause")
                {
                    WorkDeepFocus subtotal_wdf;
                    for (const auto &tw : stw.second)
                    {
                        const auto &task = tw.first;
                        const auto &wdf = tw.second;
                        {
                            auto &details = story__day__details[story][day];
                            details.first += wdf;
                            details.second.push_back(task);
                        }
                        subtotal_wdf += wdf;
                    }
                    total_wdf_day += subtotal_wdf;
                }
            }

            total_wdf += total_wdf_day;

            if (total_wdf_day.work != Duration::zero())
            {
                const double hours = round_2d(double(total_wdf_day.work.count()) / 3600.0);
                total_hours += hours;

                total_rnd_cost += round_2d(hour_rate * hours);

                if (!include_details)
                    latex_rnd << "% ";
                latex_rnd << std::fixed << std::setprecision(2);
                latex_rnd << day << " & " << AsHours{total_wdf_day.work} << " uur & " << hour_rate << " \\\\" << std::endl;
                day_count += 1;

                auto &row
                    = table.emplace_back();
                auto add = [&](const auto &v) {
                    tmp.str("");
                    tmp << v;
                    row.push_back(tmp.str());
                };
                add(day);
                add(AsHours{total_wdf_day.work});
                add(AsHours{total_wdf_day.focus});
                add(AsPct{total_wdf_day.focus, total_wdf_day.work});
                add(AsHours{total_wdf.work});
                add(AsHours{total_wdf.focus});
                add(AsPct{total_wdf.focus, total_wdf.work});
            }
        }
        total_hours = round_2d(total_hours);
        total_rnd_cost = round_2d(total_rnd_cost);
        total_power_cost = round_2d(total_power_cost);

        if (!include_details)
        {
            latex_rnd << std::fixed << std::setprecision(2);
            latex_rnd << day_count << " dagen"
                      << " & " << total_hours << " uur & " << hour_rate << " \\\\" << std::endl;
        }

        double total_cost = 0;

        latex_rnd << std::fixed << std::setprecision(2);
        latex_rnd << "{\\bf Subtotaal} & {\\bf " << total_hours << " uur} & {\\bf \\euro " << total_rnd_cost << "} \\\\*[1.5ex]" << std::endl;
        latex_rnd << "\\hline" << std::endl;
        latex << latex_rnd.str();
        total_cost += total_rnd_cost;

        if (powers.size() > 0)
        {
            double total_power = powers.back();
            if (powers.size() > 1)
                total_power -= powers.front();
            const double total_power_cost = total_power * price_per_kWh;
            latex_power << std::fixed << std::setprecision(2);
            latex_power << "{\\bf Subtotaal} & {\\bf " << total_power << " kWh} & {\\bf \\euro " << total_power_cost << "} \\\\*[1.5ex]" << std::endl;
            latex_power << "\\hline" << std::endl;
            latex << latex_power.str();
            total_cost += total_power_cost;
        }

        const auto vat = round_2d(total_cost * 0.21);
        const auto total_cost_vat = total_cost + vat;

        latex << "\\hline\\hline" << std::endl;
        latex << "{\\bf Saldo} & & {\\bf \\euro " << total_cost << "} \\\\" << std::endl;
        latex << "{\\bf BTW} & & {\\euro " << vat << "} \\\\" << std::endl;
        latex << "{\\bf Saldo met BTW} & & {\\bf \\euro " << total_cost_vat << "} \\\\" << std::endl;
        latex << "\\end{longtable}" << std::endl;
        latex << std::endl;

        os << latex.str();

        std::map<unsigned int, std::size_t> sizes;
        for (const auto &row : table)
            for (auto ix = 0u; ix < row.size(); ++ix)
                sizes[ix] = std::max(sizes[ix], row[ix].size());
        for (const auto &row : table)
        {
            for (auto ix = 0u; ix < row.size(); ++ix)
                os << row[ix] << std::string(sizes[ix] - row[ix].size() + 3, ' ');
            os << std::endl;
        }
    }

    // Timesheet::DayInfo
    void Timesheet::DayInfo::stream(std::ostream &os) const
    {
        if (!!start && !!stop)
            os << "[" << *start << " - " << *stop << "]";
    }
    void Timesheet::DayInfo::update()
    {
        if (!!start)
        {
            if (!stop)
                stop.reset(new DayTime(*start));
        }
    }
} // namespace time_track
