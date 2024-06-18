#ifndef HEADER_org_tree_Content_hpp_ALREADY_INCLUDED
#define HEADER_org_tree_Content_hpp_ALREADY_INCLUDED

#include <gubg/Strange.hpp>

#include <optional>

namespace org { namespace tree {

    class Content
    {
    public:
        std::optional<std::string> state;
        std::optional<std::string> rest;
        std::optional<std::string> tags;

        bool parse(gubg::Strange);

        bool serialize(std::string &) const;

        void reset();

    private:
    };

}} // namespace org::tree

#endif
