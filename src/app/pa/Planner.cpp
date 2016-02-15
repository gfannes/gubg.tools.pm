#include "pa/Planner.hpp"
#include "chaiscript/chaiscript.hpp"
#include "chaiscript/chaiscript_stdlib.hpp"
#include <sstream>
using namespace chaiscript;
using namespace gubg::planning;

namespace pa
{
	bool Planner::add_workers(const gubg::file::File &workers_chaiscript)
	{
        MSS_BEGIN(bool, logns);

        MSS(!workers_chaiscript.empty(), std::cout << "ERROR: You have to specify the workers file, someone has to do it, you know..." << std::endl);

        L("Loading workers from " << workers_chaiscript);

		ChaiScript chai(Std_Lib::library());
        chai.add(fun(&gubg::planning::Planning::addWorker, &planning), "add_worker");
        chai.add(fun(&Planner::set_nr_working_days, this), "set_nr_working_days");

        {
            std::ostringstream error;
            try
            {
                chai.eval_file(workers_chaiscript.name());
            }
            catch (const exception::file_not_found_error &) { error << "ERROR: Could not find workers file " << workers_chaiscript; }
            catch (const exception::eval_error &)           { error << "ERROR: Could not load workers from " << workers_chaiscript; }
            const auto error_str = error.str();
            MSS(error_str.empty(), std::cout << error_str << std::endl);
        }

        for (auto d: workDays(nr_working_days))
            planning.addDay(d);

        MSS_END();

        /* using namespace gubg::planning; */
        /* planning.addWorker("geertf", 0.8); */
        /* planning.addWorker("wimc", 0.8); */
        /* planning.addWorker("hermanh", 0.8); */
        /* planning.addWorker("emilev", 0.8); */
        /* planning.addWorker("ismaild", 0.8); */
        /* planning.addWorker("marnicr", 0.8); */
        /* for (auto d: workDays(1000)) */
        /* { */
        /* 	planning.addDay(d); */
        /* } */
#if 0
#ifdef PAST
        for (auto d: dayRange(Day(2013, 7, 4), Day(2013, 7, 18)))
            planning.absence("gfa", d);
        for (auto d: dayRange(Day(2013, 7, 22), Day(2013, 8, 2)))
            planning.absence("wba", d);
        planning.absence("wba", Day(2013, 8, 16));
        planning.absence("wba", Day(2013, 11, 28));
        planning.absence("wba", Day(2014, 1, 30));
#endif
        planning.absence("gfa", Day(2014, 3, 6));
        planning.absence("gfa", Day(2014, 3, 7));
        planning.absence("wba", Day(2014, 3, 7));
        for (auto d: dayRange(Day(2014, 8, 4), Day(2014, 8, 14)))
            planning.absence("wba", d);

        {
#ifdef PAST
            auto verlof = dayRange(Day(2013, 12, 25), Day(2014, 1, 1));
            verlof.push_back(Day(2013, 8, 15));
            verlof.push_back(Day(2013, 11, 1));
            verlof.push_back(Day(2013, 11, 11));
#endif
            auto verlof = dayRange(Day(2014, 12, 25), Day(2015, 1, 1));
            verlof.push_back(Day(2014, 5, 1));
            verlof.push_back(Day(2014, 5, 2));
            verlof.push_back(Day(2014, 5, 29));
            verlof.push_back(Day(2014, 5, 30));
            verlof.push_back(Day(2014, 6, 9));
            verlof.push_back(Day(2014, 7, 21));
            verlof.push_back(Day(2014, 8, 15));
            verlof.push_back(Day(2014, 11, 11));

            for (auto d: verlof)
            {
                planning.absence("gfa", d);
                planning.absence("wba", d);
            }
        }

        //Quality week
        for (auto d: dayRange(Day(2014, 7, 7), Day(2014, 7, 11)))
        {
            planning.absence("gfa", d);
            planning.absence("wba", d);
        }
#endif
    }
}
