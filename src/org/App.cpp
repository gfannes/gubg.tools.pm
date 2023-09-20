#include <org/App.hpp>

#include <gubg/mss.hpp>

namespace org {

    bool App::run()
    {
        MSS_BEGIN(bool);

        MSS(!!options_.filepath, log_.error() << "Expected filepath to be set" << std::endl);

        MSS_END();
    }

} // namespace org
