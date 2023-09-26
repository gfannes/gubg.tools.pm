#ifndef HEADER_org_tree_Node_hpp_ALREAD_INCLUDED
#define HEADER_org_tree_Node_hpp_ALREAD_INCLUDED

#include <org/tree/Content.hpp>
#include <org/tree/Type.hpp>
#include <org/types.hpp>

#include <gubg/ix/Range.hpp>
#include <gubg/naft/stream.hpp>

#include <variant>
#include <vector>

namespace org { namespace tree {

    class Node;

    using Nodes = std::vector<Node>;

    class Node
    {
    public:
        Type type = Type::None;
        gubg::ix::Range ix_range;
        std::variant<Nodes, Content> data;

        bool valid() const { return type != Type::None; }

        bool init(Type type);

        Nodes &childs() { return std::get<Nodes>(data); }
        const Nodes &childs() const { return std::get<Nodes>(data); }

        Content &content() { return std::get<Content>(data); }
        const Content &content() const { return std::get<Content>(data); }

        Node *find(Ix);

        void write(gubg::naft::Node &parent) const;

    private:
        static Node *find_recursive_(Ix, Node &);
    };

}} // namespace org::tree

#endif
