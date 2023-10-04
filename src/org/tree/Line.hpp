#ifndef HEADER_org_tree_Line_hpp_ALREAD_INCLUDED
#define HEADER_org_tree_Line_hpp_ALREAD_INCLUDED

#include <gubg/Strange.hpp>
#include <string>

namespace org { namespace tree {

    class Line
    {
    public:
        gubg::Strange prefix;
        gubg::Strange content;
        gubg::Strange postfix;

        bool is_bullet = false;

        std::size_t size() const { return prefix.size() + content.size() + postfix.size(); }

    private:
    };

}} // namespace org::tree

#endif
