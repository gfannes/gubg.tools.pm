#include <org/tree/Writer.hpp>

#include <gubg/mss.hpp>

namespace org { namespace tree {

    bool Writer::write(std::string &dst, const Node &node)
    {
        MSS_BEGIN(bool);
        if (const Nodes *childs = std::get_if<Nodes>(&node.data))
        {
            for (const Node &child : *childs)
            {
                MSS(write(dst, child));
            }
        }
        else if (const Content *content = std::get_if<Content>(&node.data))
        {
            dst += content->prefix.str();
            dst += content->content.str();
            dst += content->postfix.str();
        }
        MSS_END();
    }

}} // namespace org::tree
