#include <org/tree/Parser.hpp>

#include <gubg/mss.hpp>

namespace org { namespace tree {

    bool Parser::parse(const std::string &content)
    {
        MSS_BEGIN(bool);

        reset_();

        MSS(root.init(Type::Document));

        strange_ = content;
        auto &childs = root.childs();
        for (gubg::Strange line_stg, end_stg; strange_.pop_line(line_stg, end_stg);)
        {
            Node &child = childs.emplace_back();
            MSS(child.init(Type::Line));

            Line &line = child.line();
            MSS(markup_parser_.pop_prefix(line.prefix, line.is_bullet, line_stg));
            line.content = line_stg;
            line.postfix = end_stg;
        }

        // Compute the ix::Range for each node recursively
        gubg::ix::Range ix_range;
        MSS(init_ix_range_recursive_(root, ix_range));

        MSS_END();
    }

    // Privates
    void Parser::reset_()
    {
        *this = Parser(markup_type_);
    }

    bool Parser::init_ix_range_recursive_(Node &node, gubg::ix::Range &ix_range)
    {
        MSS_BEGIN(bool);

        if (auto *childs = std::get_if<Nodes>(&node.data))
        {
            for (auto &child : *childs)
            {
                MSS(init_ix_range_recursive_(child, ix_range));
            }
            node.ix_range = ix_range;
        }
        else if (auto *content = std::get_if<Line>(&node.data))
        {
            const auto my_size = content->size();
            node.ix_range = gubg::ix::Range(ix_range.stop(), my_size);
            ix_range.resize(ix_range.size() + my_size);
        }
        else
        {
            MSS(false);
        }

        MSS_END();
    }

}} // namespace org::tree
