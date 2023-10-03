#include <org/markup/Type.hpp>

#include <filesystem>

namespace org { namespace markup {

    std::ostream &operator<<(std::ostream &os, Type v)
    {
        switch (v)
        {
            case Type::Org: os << "Org"; break;
            case Type::Markdown: os << "Markdown"; break;
            default: os << "Type-" << (int)v; break;
        }
        return os;
    }

    std::optional<Type> guess_from_filepath(const std::string &fp)
    {
        const auto ext = std::filesystem::path{fp}.extension();
        if (ext == ".org")
            return Type::Org;
        if (ext == ".md")
            return Type::Markdown;
        return {};
    }

}} // namespace org::markup
