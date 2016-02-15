#ifndef HEADER_pa_Planner_hpp_ALREADY_INCLUDED
#define HEADER_pa_Planner_hpp_ALREADY_INCLUDED

#include "pa/Codes.hpp"
#include "pa/Model.hpp"
#include "gubg/file/File.hpp"
#include "gubg/tree/dfs/Iterate.hpp"
#include "gubg/planning/Task.hpp"
#include "gubg/planning/Planning.hpp"
#include <sstream>

namespace pa
{
	struct Planner
	{
        static constexpr const char *logns = "pa::Planner";

		gubg::planning::Task::Ptr root;
		gubg::planning::Task::Ptr current;
		gubg::planning::Planning planning;
		std::ostringstream parseError_;
		const std::string categoryName;
        unsigned int nr_working_days = 1000;

		Planner(const std::string &catName = ""): categoryName(catName) {}

		bool add_workers(const gubg::file::File &workers_chaiscript);
        void set_nr_working_days(unsigned int nr) {nr_working_days = nr;}

		ReturnCode run()
		{
			MSS_BEGIN(ReturnCode, logns);

			//Use model to build the gubg::planning::Task tree into root
			gubg::tree::dfs::iterate(model(), *this);
			if (!parseError_.str().empty())
			{
				std::cout << parseError_.str();
				MSS(ReturnCode::ParsingError);
			}

			{
				gubg::planning::Day day;
				MSS(planning.getLastDay(day));
                MSS(!!root);
				root->setDeadline(day);
			}
			MSS(planning.plan(*root));

			MSS_END();
		}

		template <typename Path>
			bool open(Node &n, Path &p)
			{
				S(logns);L(STREAM(n.desc, p.size(), n.total()));
				if (n.total() <= 0)
                {
                    L(STREAM(n.total()));
					return false;
                }
				if (!root)
				{
					assert(p.empty());
					//We do not put the root name, it is typically not a task and causes only bloat in the output
					current = root = gubg::planning::Task::create("");
				}
				else
					current = current->addChild(n.desc);
				assert(root);
				if (!current)
					std::cout << "ERROR::Could not add new child \"" << n.desc << "\", only a leaf node can contain sweat" << std::endl;
				assert(current);
				current->setSweat(n.value*n.fraction);
				if (n.attributes.count("allocate"))
				{
					const auto str = n.attributes["allocate"];
					gubg::Strange strange(str); 
					std::string worker;
					gubg::planning::Workers workers;
					while (strange.popUntil(worker, '|'))
					{
						if (!worker.empty())
							workers.push_back(worker);
					}
					strange.popAll(worker);
					if (!worker.empty())
						workers.push_back(worker);
					current->setWorkers(workers);
				}
				if (n.attributes.count("deadline"))
				{
					gubg::planning::Day deadline(n.attributes["deadline"]);
					if (!deadline.isValid())
						parseError_ << "Could not parse deadline attribute for node " << n.desc << ": \"" << n.attributes["deadline"] << "\"" << std::endl;
					else
						current->setDeadline(deadline);
				}
				if (n.attributes.count(categoryName) > 0)
				{
					L("Found a category");
					current->setCategory(n.attributes[categoryName]);
				}
				return true;
			}

		template <typename Path>
			void close(Node &n, Path &p)
			{
				if (current)
					current = current->parent.lock();
			}
	};
}

#endif
