#include <org/tree/Node.hpp>

#include <gubg/hex.hpp>
#include <gubg/mss.hpp>

namespace org { namespace tree {

    bool Node::init(Type type)
    {
        MSS_BEGIN(bool);

        this->type = Type::None;
        switch (type)
        {
            case Type::Document:
                data = Nodes{};
                break;
            case Type::Line:
                data = Content{};
                break;
            default:
                MSS(false);
                break;
        }
        this->type = type;

        MSS_END();
    }

    void Node::write(gubg::naft::Node &parent) const
    {
        auto n = parent.node("Node");
        n.attr("type", type);
        n.attr("ix", ix_range);
        if (const Nodes *childs = std::get_if<Nodes>(&data))
        {
            for (const auto &child : *childs)
                child.write(n);
        }
        if (const Content *content = std::get_if<Content>(&data))
        {
            auto add_attr = [&](const char *name, const auto &value) {
                if (!value.empty())
                    n.attr(name, value);
            };
            add_attr("prefix", content->prefix);
            add_attr("content", content->content);
            add_attr("postfix", gubg::hex(content->postfix.str()));
        }
    }

    Node *Node::find(Ix ix)
    {
        return find_recursive_(ix, *this);
    }

    // Privates
    Node *Node::find_recursive_(Ix ix, Node &node)
    {
        if (node.ix_range.contains(ix))
        {
            Nodes *childs = std::get_if<Nodes>(&node.data);
            if (!childs)
                return &node;

            for (auto &child : *childs)
                if (child.ix_range.contains(ix))
                    return find_recursive_(ix, child);
        }
        return nullptr;
    }

}} // namespace org::tree