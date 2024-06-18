#ifndef HEADER_org_tree_Node_hpp_ALREADY_INCLUDED
#define HEADER_org_tree_Node_hpp_ALREADY_INCLUDED

#include <org/tree/Line.hpp>
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
        std::variant<Nodes, Line> data;

        void swap(Node &);

        bool valid() const { return type != Type::None; }

        bool init(Type type);

        Nodes &childs() { return std::get<Nodes>(data); }
        const Nodes &childs() const { return std::get<Nodes>(data); }

        Line &line() { return std::get<Line>(data); }
        const Line &line() const { return std::get<Line>(data); }

        Node *find_ix(Ix ix);
        Node *find_line(Ix line);

        void write(gubg::naft::Node &parent) const;

    private:
        static Node *find_recursive_(Ix, Node &);
    };

}} // namespace org::tree

#endif
