#ifndef HEADER_pit_Types_hpp_ALREADY_INCLUDED
#define HEADER_pit_Types_hpp_ALREADY_INCLUDED

#include <gubg/Army.hpp>
#include <gubg/planning/Day.hpp>
#include <string>
#include <vector>
#include <list>
#include <ostream>
#include <optional>

namespace pit { 

    using Day = gubg::planning::Day;

    using Tag = std::string;

    struct InputFile
    {
        std::string ns;
        std::string fn;
    };
    using InputFiles = std::vector<InputFile>;

    enum class Moscow { Must, Should, Could, Wont, };

    struct Data
    {
        Tag tag;
        std::optional<std::string> name;
        std::string note;
        std::optional<Moscow> moscow;
        gubg::Army agg_duration;
        gubg::Army agg_todo;
        std::optional<gubg::Army> duration;
        std::optional<gubg::Army> todo;
        std::optional<std::string> deadline;
        std::string story;
        std::list<std::string> deps;
        std::list<std::string> belongs;
        std::optional<std::string> ui_required_skill;
        std::string required_skill;
        std::optional<std::string> key;
        bool sequential = false;

        std::optional<std::string> worker;
        Day first;
        Day last;
        std::optional<Day> agg_first;
        std::optional<Day> agg_last;

        Data(){}
        Data(const std::string &tag): tag(tag) {}
    };
    inline std::ostream &operator<<(std::ostream &os, const Data &data)
    {
        os << "[data](tag:" << data.tag << ")";
        return os;
    }

} 

#endif
