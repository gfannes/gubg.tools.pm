#include <org/markup/Parser.hpp>

#include <gubg/mss.hpp>

#include <optional>

namespace org { namespace markup {

    bool Parser::pop_prefix(gubg::Strange &prefix, bool &is_bullet, gubg::Strange &line) const
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
                    for (char ch : {'*', '-'})
                        if (my_line.pop_if(ch) && my_line.pop_if(' '))
                        {
                            count = line.size() - my_line.size();
                            is_bullet = true;
                            break;
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

    bool Parser::extract_link(gubg::Strange &text, gubg::Strange &link, gubg::Strange line) const
    {
        MSS_BEGIN(bool);

        switch (type_)
        {
            case Type::Markdown:
                MSS_Q(line.pop_to('['));
                MSS(line.pop_bracket(text, "[]"));
                MSS(line.pop_bracket(link, "()"));
                break;
            case Type::Org:
                MSS_Q(line.pop_until('['));
                MSS(line.pop_bracket(link, "[]"));
                MSS(line.pop_bracket(text, "[]"));
                break;
            default: MSS(false); break;
        }

        MSS_END();
    }

}} // namespace org::markup
