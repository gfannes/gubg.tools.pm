#ifndef HEADER_pit_ResourceMgr_hpp_ALREADY_INCLUDED
#define HEADER_pit_ResourceMgr_hpp_ALREADY_INCLUDED

#include <pit/Types.hpp>
#include <pit/Resource.hpp>
#include <gubg/mss.hpp>

namespace pit { 

    class ResourceMgr
    {
    public:
        bool get(Resources &resources, const Day &day) const
        {
            MSS_BEGIN(bool);
            MSS_END();
        }
    private:
    };

} 

#endif
