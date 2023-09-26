#ifndef HEADER_org_tree_Prefix_hpp_ALREAD_INCLUDED
#define HEADER_org_tree_Prefix_hpp_ALREAD_INCLUDED

#include <gubg/Strange.hpp>

#include <optional>

namespace org { namespace tree {

    class Prefix
    {
    public:
        std::optional<std::string> indent;
        std::optional<std::string> state;
        std::optional<std::string> rest;

        bool parse(gubg::Strange);

        bool serialize(std::string &) const;

    private:
    };

}} // namespace org::tree

#endif
