#include "pa/Codes.hpp"
#include "pa/Tasks.hpp"
#include "pa/Finalize.hpp"
#include "gubg/OptionParser.hpp"
using namespace pa;
using gubg::OptionParser;
using namespace std;

namespace 
{
    ReturnCode main_(int argc, const char **argv)
    {
        MSS_BEGIN(ReturnCode);

        Tasks tasks;
        Options options;
		{
			bool loadMindMap = false;
			OptionParser optionParser("Develop assistent");
			optionParser.add_switch('h', "--help", "Displays this help", [&optionParser](){PA_FINALIZE_OK(optionParser.help());});
			optionParser.add_mandatory('i', "--input FILE", "Input file to use", [&options](string file){options.input = file;});
			optionParser.add_mandatory('w', "--workers FILE", "Workers chaiscript file to use", [&options](string file){options.workers = file;});
			optionParser.add_mandatory('o', "--output FILE", "Output file to use", [&options](string file){options.output = file;});
			optionParser.add_mandatory('f', "--fraction NAME", "Use freeplane attribute NAME as fraction", [&options](string name){options.fraction = name;});
			optionParser.add_mandatory('F', "--fraction-default VALUE", "Default fraction to use", [&options](string value){options.fraction_default = value;});
			optionParser.add_mandatory('l', "--line NAME", "Line NAME to use", [&options](string name){options.lines.push_back(name);});
			optionParser.add_mandatory('t', "--totals NAME", "The totals to be used",
					[&options, &tasks, &loadMindMap](string name){options.value = name; loadMindMap = true;});
			optionParser.add_mandatory('T', "--totals-to-days VALUE", "Factor to convert the totals to days", [&options](string value){
                    if (!options.value2days.set(value))
                        PA_FINALIZE_OK("Could not understand the value2days");
                    });
			optionParser.add_mandatory('c', "--category NAME", "The category to be used for quarter reporting",
					[&options, &tasks, &loadMindMap](string name){options.category = name; loadMindMap = true;});
			optionParser.add_switch('d', "--details", "Show detailed work breakdown",
					[&options, &tasks]()
					{
					if (options.value.empty()) PA_FINALIZE_OK("You have to specify the totals to be used before the details");
					tasks.push_back(ShowWBS::create());
					});
			optionParser.add_switch('p', "--plan-overview", "Planning overview",
					[&options, &tasks]()
					{
					if (options.value.empty()) PA_FINALIZE_OK("You have to specify the totals to be used before the planning overview");
					tasks.push_back(Plan::create(Plan::Overview));
					});
			optionParser.add_switch('P', "--plan-details", "Planning details",
					[&options, &tasks]()
					{
					if (options.value.empty()) PA_FINALIZE_OK("You have to specify the totals to be used before the planning details");
					tasks.push_back(Plan::create(Plan::Details));
					});
			optionParser.add_switch('x', "--plan-products", "Products details",
					[&options, &tasks]()
					{
					if (options.value.empty()) PA_FINALIZE_OK("You have to specify the totals to be used before the planning products");
					tasks.push_back(Plan::create(Plan::Products));
					});
			optionParser.add_switch('r', "--resource", "Resource view of the planning per month",
					[&options, &tasks]()
					{
					if (options.value.empty()) PA_FINALIZE_OK("You have to specify the totals to be used before the planning products");
					tasks.push_back(Plan::create(Plan::Resource));
					});
			optionParser.add_switch('q', "--quarter", "Quarter view of the planning",
					[&options, &tasks]()
					{
					if (options.value.empty()) PA_FINALIZE_OK("You have to specify the totals to be used before the quarter view");
					if (options.category.empty()) PA_FINALIZE_OK("You have to specify the category to be used before the quarter view");
					tasks.push_back(Quarter::create());
					});
			optionParser.add_switch('D', "--debug", "Debug view of the tree",
					[&options, &tasks]()
					{
					if (options.value.empty()) PA_FINALIZE_OK("You have to specify the totals to be used before the debug view");
					tasks.push_back(ShowDebug::create());
					});

			auto args = OptionParser::create_args(argc, argv);
			MSS(optionParser.parse(args));
			if (loadMindMap)
				tasks.push_front(LoadMindMap::create());
		}

        for (auto task: tasks)
            MSS(task->execute(options));

        MSS_END();
    }
}

int main(int argc, const char **argv)
{
    if (!gubg::mss::is_ok(main_(argc, argv)))
        PA_FINALIZE_ERROR("Problems were encountered...");
    PA_FINALIZE_OK("Everything went OK");
    return 0;
}
