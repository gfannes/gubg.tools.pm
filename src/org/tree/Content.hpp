#ifndef HEADER_org_tree_Content_hpp_ALREAD_INCLUDED
#define HEADER_org_tree_Content_hpp_ALREAD_INCLUDED

#include <gubg/Strange.hpp>
#include <string>

namespace org { namespace tree {

    class Content
    {
    public:
        gubg::Strange prefix;
        gubg::Strange content;
        gubg::Strange postfix;

        std::size_t size() const { return prefix.size() + content.size() + postfix.size(); }

    private:
    };

}} // namespace org::tree

#endif
