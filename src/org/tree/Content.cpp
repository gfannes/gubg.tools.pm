#include <org/tree/Content.hpp>

#include <gubg/mss.hpp>
#include <gubg/string/concat.hpp>

namespace org { namespace tree {

    void Content::reset()
    {
        *this = Content();
    }

    bool Content::parse(gubg::Strange strange)
    {
        MSS_BEGIN(bool);

        reset();

        // State
        {
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
        }

        // Rest and Tags
        {
            std::optional<std::size_t> start_ix;
            if (!strange.empty() && strange.back() == ':')
            {
                for (auto ix : gubg::ix::make_range(strange.size() - 1).reverse())
                {
                    const char ch = strange[ix];
                    if (ch == ':')
                        start_ix = ix;
                    else if (ch == ' ')
                        break;
                }
            }

            if (start_ix)
            {
                MSS(strange.pop_count(rest.emplace(), *start_ix));
                tags = strange.str();
            }
            else
            {
                rest = strange.str();
            }
        }

        MSS_END();
    }

    bool Content::serialize(std::string &dst) const
    {
        MSS_BEGIN(bool);
        dst.resize(0);
        auto append = [&](const std::optional<std::string> &str_opt) {
            if (str_opt)
            {
                if (!dst.empty() && dst[dst.size() - 1] != ' ')
                    dst.push_back(' ');
                dst += *str_opt;
            }
        };
        append(state);
        append(rest);
        append(tags);
        MSS_END();
    }

}} // namespace org::tree
