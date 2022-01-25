#include <time_track/ReturnCode.hpp>
#include <time_track/Options.hpp>
#include <time_track/Timesheet.hpp>

#include <gubg/Logger.hpp>

#include <iostream>
#include <functional>
#include <optional>

namespace time_track { 
    ReturnCode main(int argc, const char **argv)
    {
        MSS_BEGIN(ReturnCode, "time_track");

        gubg::Logger log;

        Options options;
        MSS(options.parse(argc, argv, log));

        if (options.print_help)
        {
            log.os(0) << options.help();
            return ReturnCode::OK;
        }

        MSS(!options.input_fn.empty(), log.error() << "No input file given" << std::endl);

        Timesheet timesheet{log};
        if (options.from && options.from->year >= 0 && options.from->month >= 0)
            timesheet.filter_from(options.from->year, options.from->month, options.from->day);
        if (options.until && options.until->year >= 0 && options.until->month >= 0)
            timesheet.filter_until(options.until->year, options.until->month, options.until->day);
        MSS(timesheet.parse(options.input_fn));
        log.os(0) << timesheet;
        if (options.print_totals)
            timesheet.stream_totals(log.os(0));

        MSS_END();
    }
} 

int main(int argc, const char **argv)
{
    const auto rc = time_track::main(argc, argv);
    switch (rc)
    {
        case time_track::ReturnCode::OK: std::cout << "Everything went OK" << std::endl; break;
        default: std::cerr << "Failure: " << C(rc, int) << std::endl; break;
    }
    return 0;
}
