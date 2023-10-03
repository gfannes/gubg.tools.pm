#ifndef HEADER_org_markup_Type_hpp_ALREAD_INCLUDED
#define HEADER_org_markup_Type_hpp_ALREAD_INCLUDED

#include <ostream>
#include <optional>

namespace org { namespace markup {

    enum class Type
    {
        Org,
        Markdown,
    };

    std::ostream &operator<<(std::ostream &, Type);

    std::optional<Type> guess_from_filepath(const std::string &);

}} // namespace org::markup

#endif
