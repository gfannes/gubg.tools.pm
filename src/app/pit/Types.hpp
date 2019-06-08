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

    enum class Mode
    {
        Report,
        Plan,
    };
    inline std::ostream &operator<<(std::ostream &os, Mode m)
    {
        switch (m)
        {
            case Mode::Report: os << "report"; break;
            case Mode::Plan: os << "plan"; break;
        }
        return os;
    }

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
        std::string note;
        std::optional<Moscow> moscow;
        gubg::Army total_duration;
        gubg::Army total_todo;
        std::optional<gubg::Army> duration;
        std::optional<gubg::Army> todo;
        std::optional<std::string> deadline;
        std::string story;
        std::list<std::string> deps;
        std::string required_skill;

        std::optional<std::string> worker;
        Day first;
        Day last;

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
