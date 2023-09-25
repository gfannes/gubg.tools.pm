#ifndef HEADER_org_tree_Type_hpp_ALREAD_INCLUDED
#define HEADER_org_tree_Type_hpp_ALREAD_INCLUDED

#include <ostream>

namespace org { namespace tree {

    enum class Type
    {
        None,
        Document,
        Line,
    };

    std::ostream &operator<<(std::ostream &, Type);

}} // namespace org::tree

#endif
