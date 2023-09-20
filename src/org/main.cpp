#include <org/App.hpp>

#include <gubg/Logger.hpp>
#include <gubg/mss.hpp>

namespace org {
    bool main(int argc, const char **argv)
    {
        MSS_BEGIN(bool);

        Options options;
        MSS(options.init(argc, argv));
        MSS(options.init(EnvVars::Helix));
        std::cout << options << std::endl;

        App app{options};

        MSS(app.run());

        MSS_END();
    }
} // namespace org

int main(int argc, const char **argv)
{
    gubg::Logger log;

    if (!org::main(argc, argv))
    {
        log.error() << "Something went wrong" << std::endl;
        return -1;
    }

    log.os(0) << "Everything went OK" << std::endl;
    return 0;
}
