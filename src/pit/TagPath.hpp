#ifndef HEADER_pit_TagPath_hpp_ALREADY_INCLUDED
#define HEADER_pit_TagPath_hpp_ALREADY_INCLUDED

#include <pit/Types.hpp>
#include <gubg/string_algo/algo.hpp>
#include <gubg/OnlyOnce.hpp>
#include <gubg/Range.hpp>
#include <vector>
#include <algorithm>
#include <sstream>

namespace pit { 

    class TagPath: public std::vector<Tag>
    {
    public:
        static const char sep() {return '/';}

        template <typename Node>
        TagPath(const Node &node)
        {
            for (const Node *ptr = &node; ptr; ptr = ptr->parent().get())
                push_back(ptr->tag);
            std::reverse(RANGE(*this));
        }
        TagPath(const std::string &str)
        {
            gubg::string_algo::split(*this, str, sep());
        }
        TagPath(const TagPath &a, const TagPath &b)
        {
            insert(end(), RANGE(a));
            insert(end(), RANGE(b));
        }

        void stream(std::ostream &os) const
        {
            gubg::OnlyOnce skip_sep;
            for (const auto &e: *this)
            {
                if (!skip_sep())
                    os << sep();
                os << e;
            }
        }
        std::string str() const
        {
            oss_.str("");
            stream(oss_);
            return oss_.str();
        }
    private:
        static std::ostringstream oss_;
    };

    inline std::ostream &operator<<(std::ostream &os, const TagPath &tp)
    {
        tp.stream(os);
        return os;
    }

} 

#endif
