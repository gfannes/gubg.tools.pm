#ifndef HEADER_org_tree_Parser_hpp_ALREAD_INCLUDED
#define HEADER_org_tree_Parser_hpp_ALREAD_INCLUDED

#include <org/tree/Node.hpp>

#include <org/markup/Type.hpp>
#include <org/markup/Parser.hpp>

#include <gubg/Strange.hpp>
#include <gubg/ix/Range.hpp>

#include <string>

namespace org { namespace tree {

    class Parser
    {
    public:
        Node root;

        bool init(markup::Type markup_type);

        bool parse(const std::string &);

    private:
        void reset_();
        static bool init_ix_range_recursive_(Node &, gubg::ix::Range &);

        markup::Type markup_type_ = markup::Type::None;
        markup::Parser markup_parser_;
        gubg::Strange strange_;
    };

}} // namespace org::tree

#endif
