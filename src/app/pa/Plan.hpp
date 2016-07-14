#ifndef HEADER_pa_Plan_hpp_ALREADY_INCLUDED
#define HEADER_pa_Plan_hpp_ALREADY_INCLUDED

#include "pa/Tasks.hpp"
#include "pa/Types.hpp"

namespace pa
{
    class Plan: public ITask
    {
        public:
            enum View {Overview, Products, Details, Resource};

            static Ptr create(View view){return Ptr(new Plan(view));}

            virtual ReturnCode execute(const Options &);

        private:
            Plan(View view):view_(view){}
            const View view_;
    };
}

#endif
