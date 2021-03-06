#ifndef HEADER_pa_Codes_hpp_ALREADY_INCLUDED
#define HEADER_pa_Codes_hpp_ALREADY_INCLUDED

#include "gubg/mss.hpp"

namespace pa
{
    enum class ReturnCode
    {
        OK, Error,

        UnknownExtension,
		ParsingError,
    };
}

#endif
