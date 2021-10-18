#include "time_track/ReturnCode.hpp"
#include "time_track/Options.hpp"
#include "time_track/Timesheet.hpp"
#include "gubg/OptionParser.hpp"
#include "gubg/macro/capture.hpp"
#include <iostream>
#include <functional>
using namespace std;

namespace time_track { 
    ReturnCode main(int argc, const char **argv)
    {
        MSS_BEGIN(ReturnCode, "time_track");

        Options options;

        gubg::OptionParser parser("Timesheet tracker");
        parser.add_switch('h', "--help", "Print this help", [&](){options.print_help = true;});
        parser.add_switch('t', "--totals", "Print totals", [&](){options.print_totals = true;});
        parser.add_mandatory('i', "--input", "Time naft", [&](const std::string &str){options.input_fn = str;});
        auto parse_filter = [&](std::optional<Options::YYYYMMDD> &dst_opt, const std::string &str)
        {
            auto &dst = dst_opt.emplace();
            const auto v = std::stoi(str);
            switch (str.size())
            {
                case 6:
                    dst.year = v/100;
                    dst.month = v%100;
                    dst.day = 1;
                    break;
                case 8:
                    dst.year = v/10000;
                    dst.month = (v/100)%100;
                    dst.day = v%100;
                    break;
                default:
                    std::cout << "Could not parse filter " << str << std::endl;
                    break;
            }
        };
        parser.add_mandatory('f', "--from", "Remove info strictly before YYYYMM[DD]", std::bind(parse_filter, std::ref(options.from),  std::placeholders::_1));
        parser.add_mandatory('u', "--until", "Remove info after and including YYYYMM[DD]", std::bind(parse_filter, std::ref(options.until), std::placeholders::_1));

        auto args = gubg::OptionParser::create_args(argc, argv);
        MSS(parser.parse(args));

        if (options.print_help)
        {
            cout << parser.help();
        }
        else if (options.input_fn.empty())
        {
            cerr << "No input file given" << endl;
        }
        else
        {
            Timesheet timesheet;
            if (options.from && options.from->year >= 0 && options.from->month >= 0)
                timesheet.filter_from(options.from->year, options.from->month, options.from->day);
            if (options.until && options.until->year >= 0 && options.until->month >= 0)
                timesheet.filter_until(options.until->year, options.until->month, options.until->day);
            MSS(timesheet.parse(options.input_fn));
            cout << timesheet;
            if (options.print_totals)
                timesheet.stream_totals(cout);
        }

        MSS_END();
    }
} 

int main(int argc, const char **argv)
{
    const auto rc = time_track::main(argc, argv);
    switch (rc)
    {
        case time_track::ReturnCode::OK:
            cout << "Everything went OK" << endl;
            break;
        default:
            cerr << C(rc, int) << endl;
            break;
    }
    return 0;
}
