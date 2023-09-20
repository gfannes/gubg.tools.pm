#ifndef HEADER_pit_Types_hpp_ALREADY_INCLUDED
#define HEADER_pit_Types_hpp_ALREADY_INCLUDED

#include <pit/Log.hpp>
#include <pit/Day.hpp>
#include <gubg/Army.hpp>
#include <string>
#include <vector>
#include <list>
#include <ostream>
#include <optional>

namespace pit { 

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
        bool done = false;
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
        Day_opt not_before;
        Day_opt first;
        Day_opt last;
        Day_opt agg_first;
        Day_opt agg_last;

        Data(){}
        Data(const std::string &tag): tag(tag) {}

        void stream(std::ostream &os) const
        {
            os << "[data](tag:" << tag << ")";
            if (agg_last)
                os << "(agg_last:" << *agg_last << ")";
            if (not_before)
                os << "(not_before:" << *not_before << ")";
        }
    };
    inline std::ostream &operator<<(std::ostream &os, const Data &data)
    {
        data.stream(os);
        return os;
    }

} 

#endif
