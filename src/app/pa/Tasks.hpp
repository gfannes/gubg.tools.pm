#ifndef HEADER_pa_Tasks_hpp_ALREADY_INCLUDED
#define HEADER_pa_Tasks_hpp_ALREADY_INCLUDED

#include "pa/Codes.hpp"
#include "pa/Options.hpp"
#include <list>
#include <memory>

namespace pa {

    class ITask
    {
        public:
            typedef std::shared_ptr<ITask> Ptr;
            virtual ReturnCode execute(const Options &) = 0;
    };
    typedef std::list<ITask::Ptr> Tasks;

}

#include "pa/LoadMindMap.hpp"
#include "pa/ShowWBS.hpp"
#include "pa/ShowDebug.hpp"
#include "pa/Plan.hpp"
#include "pa/Quarter.hpp"

#endif
