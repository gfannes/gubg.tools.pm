#include <org/tree/Prefix.hpp>

#include <gubg/mss.hpp>

namespace org { namespace tree {

    bool Prefix::parse(gubg::Strange strange)
    {
        MSS_BEGIN(bool);

        for (auto ch : {'#', '*'})
        {
            if (const auto count = strange.strip_left(ch); count > 0)
            {
                std::cout << "Found " << count << ' ' << ch << std::endl;
                indent.emplace(count, ch);
                break;
            }
        }

        strange.strip_left(' ');

        for (auto some_state : {"TODO", "NEXT"})
            if (strange.pop_if(some_state) && strange.pop_if(' '))
                state = some_state;

        rest = strange.str();

        MSS_END();
    }

    bool Prefix::serialize(std::string &dst) const
    {
        MSS_BEGIN(bool);
        auto append = [&](const std::optional<std::string> &str_opt) {
            if (str_opt)
            {
                dst += *str_opt;
                dst.push_back(' ');
            }
        };
        append(indent);
        append(state);
        append(rest);
        MSS_END();
    }

}} // namespace org::tree
