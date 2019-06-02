#include "time_track/ReturnCode.hpp"
#include "time_track/Options.hpp"
#include "time_track/Timesheet.hpp"
#include "gubg/OptionParser.hpp"
#include "gubg/macro/capture.hpp"
#include <iostream>
using namespace std;

namespace time_track { 
    ReturnCode main(int argc, const char **argv)
    {
        MSS_BEGIN(ReturnCode, "time_track");

        Options options;

        gubg::OptionParser parser("Timesheet tracker");
        parser.add_switch('h', "--help", "Print this help", [&](){options.print_help = true;});
        parser.add_mandatory('i', "--input", "Time naft", [&](const std::string &str){options.input_fn = str;});
        auto parse_filter = [&](const std::string &str)
        {
            const auto v = std::stoi(str);
            switch (str.size())
            {
                case 6:
                    options.year = v/100;
                    options.month = v%100;
                    options.day = 1;
                    break;
                case 8:
                    options.year = v/10000;
                    options.month = (v/100)%100;
                    options.day = v%100;
                    break;
                default:
                    std::cout << "Could not parse filter " << str << std::endl;
                    break;
            }
        };
        parser.add_mandatory('f', "--filter", "Filter everything before YYYYMM[DD]", parse_filter);

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
            if (options.year >= 0 && options.month >= 0)
                timesheet.filter(options.year, options.month, options.day);
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
