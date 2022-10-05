#include <time_track/Options.hpp>

#include <gubg/mss.hpp>

#include <termcolor/termcolor.hpp>

#include <sstream>
#include <iomanip>

namespace time_track { 


    bool Options::parse(int argc, const char **argv, gubg::Logger &log)
    {
        MSS_BEGIN(bool);

        auto parse_yyyymmdd = [&](YYYYMMDD &dst, const std::string &str){
            MSS_BEGIN(bool);
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

                default: MSS(false, log.error() << "Could not parse YYYYMMDD " << str << std::endl); break;
            }
            MSS_END();
        };

        unsigned int arg_ix = 0;
        auto pop_arg = [&](std::string &str){
            if (arg_ix == argc)
                return false;
            str = argv[arg_ix++];
            return true;
        };

        MSS(pop_arg(exe_name), log.error() << "Could not find exe_name" << std::endl);

        for (std::string arg; pop_arg(arg); )
        {
            auto is = [&](const char *sh, const char *lh){ return arg == sh || arg == sh; };
            std::string tmp;

            if (false);
            else if (is("-h", "--help")){print_help = true;}
            else if (is("-t", "--totals")){print_totals = true;}
            else if (is("-i", "--input")){MSS(pop_arg(input_fn), log.error() << "Expected input filename" << std::endl);}
            else if (is("-V", "--verbose"))
            {
                MSS(pop_arg(tmp), log.error() << "Expected verbosity level for --verbose" << std::endl);
                verbose_level = std::stoi(tmp);
            }
            else if (is("-f", "--from"))
            {
                MSS(pop_arg(tmp), log.error() << "Expected YYYYMMDD for --from" << std::endl);
                MSS(parse_yyyymmdd(from.emplace(), tmp), log.error() << "Could not interpret YYYYMMDD for --from" << std::endl);
            }
            else if (is("-u", "--until"))
            {
                MSS(pop_arg(tmp), log.error() << "Expected YYYYMMDD for --until" << std::endl);
                MSS(parse_yyyymmdd(until.emplace(), tmp), log.error() << "Could not interpret YYYYMMDD for --until" << std::endl);
            }
            else if (is("-r", "--hour-rate"))
            {
                MSS(pop_arg(tmp), log.error() << "Expected hour rate to be set" << std::endl);
                try
                {
                    hour_rate = std::stod(tmp);
                }
                catch (...)
                {
                    MSS(false, log.error() << "Could not interpret hour rate from '" << tmp << "'" << std::endl);
                }
            }
            else MSS(false, log.error() << "Unknown argument " << arg << std::endl);
        }

        MSS_END();
    }

    std::string Options::help() const
    {
        std::ostringstream oss; oss << termcolor::colorize;

        oss << "Usage: " << termcolor::green << exe_name << " option**" << termcolor::reset << std::endl;

        auto option = [&](auto sh, auto lh, auto opt, auto expl){
            oss                      << std::setw(4)               << " ";
            oss << termcolor::yellow << std::setw(6)  << std::left << sh << termcolor::reset;
            oss << termcolor::yellow << std::setw(12) << std::left << lh << termcolor::reset;
            oss << termcolor::blue   << std::setw(16) << std::left << opt << termcolor::reset;
            oss                      << std::setw(6)  << std::left << expl;
            oss << std::endl;
        };

        oss << "Options:" << std::endl;
        option("-h", "--help", "", "Print this help");
        option("-V", "--verbose", "<NUMBER>", "Set verbosity level [default 0]");
        option("-i", "--input", "<FILEPATH>", "Set input filepath");
        option("-f", "--from", "<YYYYMM(DD)?>", "Start timestamp");
        option("-u", "--until", "<YYYYMM(DD)?>", "End timestamp, non-inclusive");
        option("-t", "--totals", "", "Print totals");
        option("-r", "--hour-rate", "PRICE", "Hour rate");
        oss << "Written by Geert Fannes" << std::endl;
        return oss.str();
    }
} 