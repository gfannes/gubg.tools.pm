#include <org/markup/Parser.hpp>

#include <gubg/mss.hpp>

#include <optional>

namespace org { namespace markup {

    bool Parser::pop_prefix(gubg::Strange &prefix, bool &is_bullet, gubg::Strange &line)
    {
        MSS_BEGIN(bool);

        is_bullet = false;

        std::optional<std::size_t> count;
        gubg::Strange my_line = line;
        switch (type_)
        {
            case Type::Markdown:
            {
                if (my_line.strip_left('#') && my_line.pop_if(' '))
                    count = line.size() - my_line.size();
                else
                {
                    my_line.strip_left(' ');
                    if (my_line.pop_if('*') && my_line.pop_if(' '))
                    {
                        count = line.size() - my_line.size();
                        is_bullet = true;
                    }
                }
            }
            break;
            case Type::Org:
                if (my_line.strip_left('*') && my_line.pop_if(' '))
                    count = line.size() - my_line.size();
                else
                {
                    my_line.strip_left(' ');
                    if (my_line.pop_if('-') && my_line.pop_if(' '))
                    {
                        count = line.size() - my_line.size();
                        is_bullet = true;
                    }
                }
                break;
            default: MSS(false); break;
        }

        if (count)
            MSS(line.pop_count(prefix, *count));

        MSS_END();
    }

}} // namespace org::markup
