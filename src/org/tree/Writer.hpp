#ifndef HEADER_org_tree_Writer_hpp_ALREAD_INCLUDED
#define HEADER_org_tree_Writer_hpp_ALREAD_INCLUDED

#include <org/tree/Node.hpp>

namespace org { namespace tree {

    class Writer
    {
    public:
        bool write(std::string &dst, const Node &);

    private:
    };

}} // namespace org::tree

#endif
