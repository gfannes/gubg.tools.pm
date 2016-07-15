#include "tt/ReturnCode.hpp"
#include "tt/Options.hpp"
#include "tt/Timesheet.hpp"
#include "gubg/OptionParser.hpp"
#include "gubg/macro/capture.hpp"
#include <iostream>
using namespace std;

namespace tt { 
    ReturnCode main(int argc, const char **argv)
    {
        MSS_BEGIN(ReturnCode);

        Options options;

        gubg::OptionParser parser("Timesheet tracker");
        parser.add_switch("-h", "--help", "Print this help", [&](){options.print_help = true;});
        parser.add_mandatory("-i", "--input", "Time tree", [&](const std::string &str){options.input_fn = str;});

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
            MSS(timesheet.parse(options.input_fn));
            cout << timesheet;
        }

        MSS_END();
    }
} 

int main(int argc, const char **argv)
{
    const auto rc = tt::main(argc, argv);
    switch (rc)
    {
        case tt::ReturnCode::OK:
            cout << "Everything went OK" << endl;
            break;
        default:
            cerr << C(rc, int) << endl;
            break;
    }
    return 0;
}
