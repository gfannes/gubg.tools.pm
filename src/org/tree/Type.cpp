#include <org/tree/Type.hpp>

namespace org { namespace tree {

    std::ostream &operator<<(std::ostream &os, Type v)
    {
        switch (v)
        {
            case Type::None: os << "None"; break;
            case Type::Document: os << "Document"; break;
            case Type::Line: os << "Line"; break;
            default: os << "Type-" << (int)v; break;
        }
        return os;
    }

}} // namespace org::tree
