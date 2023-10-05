#ifndef HEADER_org_markup_Parser_hpp_ALREAD_INCLUDED
#define HEADER_org_markup_Parser_hpp_ALREAD_INCLUDED

#include <org/markup/Type.hpp>

#include <gubg/Strange.hpp>

namespace org { namespace markup {

    class Parser
    {
    public:
        Parser(Type type)
            : type_(type) {}

        bool pop_prefix(gubg::Strange &prefix, bool &is_bullet, gubg::Strange &line) const;

        bool extract_link(gubg::Strange &text, gubg::Strange &link, gubg::Strange line) const;

    private:
        Type type_;
    };

}} // namespace org::markup

#endif
