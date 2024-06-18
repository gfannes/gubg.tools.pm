#ifndef HEADER_org_markup_Type_hpp_ALREADY_INCLUDED
#define HEADER_org_markup_Type_hpp_ALREADY_INCLUDED

#include <ostream>
#include <optional>

namespace org { namespace markup {

    enum class Type
    {
        None,
        Org,
        Markdown,
    };

    std::ostream &operator<<(std::ostream &, Type);

    std::optional<Type> guess_from_filepath(const std::string &);

}} // namespace org::markup

#endif
