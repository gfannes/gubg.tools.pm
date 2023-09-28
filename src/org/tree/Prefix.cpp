#include <org/tree/Prefix.hpp>

#include <gubg/mss.hpp>
#include <gubg/string/concat.hpp>

namespace org { namespace tree {

    void Prefix::reset()
    {
        *this = Prefix();
    }

    bool Prefix::parse(gubg::Strange strange)
    {
        MSS_BEGIN(bool);

        reset();

        auto strip_and_append = [&](char ch) {
            if (const auto count = strange.strip_left(ch); count > 0)
            {
                if (!indent)
                    indent.emplace();
                indent->append(count, ch);
                return true;
            }
            return false;
        };

        strip_and_append(' ');

        for (auto ch : {'#', '*', '-'})
            if (strip_and_append(ch))
                break;

        strange.strip_left(' ');

        for (auto some_state : {"TODO", "QUESTION", "ACTIVE", "NEXT", "FOCUS", "CHECK", "REWORK", "WAITING", "BLOCKED", "CANCELED", "DONE"})
            if (!state)
            {
                const auto sp = strange;
                if (strange.pop_if(some_state) && strange.pop_if(' '))
                    state = some_state;
                else
                    strange = sp;
            }

        gubg::Strange tmp;
        if (!state)
        {
            const auto sp = strange;
            if (strange.pop_bracket(tmp, "[]") && tmp.size() <= 1 && strange.pop_if(' '))
                state = gubg::string::concat('[', tmp.str(), ']');
            else
                strange = sp;
        }

        rest = strange.str();

        MSS_END();
    }

    bool Prefix::serialize(std::string &dst) const
    {
        MSS_BEGIN(bool);
        auto append = [&](const std::optional<std::string> &str_opt) {
            if (str_opt)
            {
                if (!dst.empty() && dst[dst.size() - 1] != ' ')
                    dst.push_back(' ');
                dst += *str_opt;
            }
        };
        append(indent);
        append(state);
        append(rest);
        MSS_END();
    }

}} // namespace org::tree
