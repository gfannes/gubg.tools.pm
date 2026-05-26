#include <org/run/LSP.hpp>
#include <org/run/Normal.hpp>

#include <gubg/Logger.hpp>
#include <gubg/mss.hpp>

int main(int argc, const char **argv)
{
    MSS_BEGIN(int);

    gubg::Logger log;

    org::Options options;
    MSS(options.init(argc, argv, log), log.error() << "Could not interpret all CLI arguments" << std::endl);
    if (!log.to_file(options.log_filepath))
        log.warning() << "Cannot log to '" << options.log_filepath << "'" << std::endl;
    log.level = options.verbose;
    log.os(1) << options << std::endl;

    MSS(options.init(org::EnvVars::Helix, log), log.error() << "Could not find expected Helix environment variables" << std::endl);

    if (options.print_help)
        log.os(log.level) << options.help();
    else
        switch (options.mode)
        {
            case org::Mode::Normal:
            {
                org::run::Normal app{options, log};
                MSS(app.run());
            }
            break;
            case org::Mode::LSP:
            {
                org::run::LSP app{options, log};
                MSS(app.run());
            }
            break;
        }

    log.os(0) << "Everything went OK" << std::endl;

    MSS_END();
}
