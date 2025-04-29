#include <org/markup/Parser.hpp>

#include <gubg/mss.hpp>

#include <optional>

namespace org { namespace markup {

    bool Parser::init(Type type)
    {
        MSS_BEGIN(bool);
        type_ = type;
        MSS_END();
    }

    bool Parser::pop_prefix(gubg::Strange &prefix, bool &is_bullet, gubg::Strange &line) const
    {
        MSS_BEGIN(bool);

        MSS(type_ != Type::None);

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
                    my_line.strip_left(" \t");
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
        switch (type_)
        {
            case Type::Markdown:
                // Format: [text](link)
                while (line.pop_to('['))
                {
                    if (!line.pop_bracket(text, "[]"))
                        continue;
                    if (!line.pop_bracket(link, "()"))
                        continue;
                    // Found link
                    return true;
                }
                break;
            case Type::Org:
                // Format: [[link][text]]
                while (line.pop_to('['))
                {
                    gubg::Strange my_line;
                    if (!line.pop_bracket(my_line, "[]"))
                        continue;
                    if (!my_line.pop_bracket(link, "[]"))
                        continue;
                    link.pop_if("file:");
                    if (!my_line.pop_bracket(text, "[]"))
                        continue;
                    // Found link
                    return true;
                }
                break;
            case Type::None: break;
        }

        // Could not find link
        return false;
    }

}} // namespace org::markup
