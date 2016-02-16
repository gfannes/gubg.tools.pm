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

        using Task_ptr = gubg::planning::Task::Ptr;
        Task_ptr root;
		Task_ptr current;
		gubg::planning::Planning planning;
		std::ostringstream parseError_;
		const std::string categoryName;
        unsigned int nr_working_days = 1000;
        using Tag = std::string;
        using TaskPerTag = std::map<Tag, Task_ptr>;
        TaskPerTag task_per_tag;
        using Tags = std::vector<std::string>;
        using Dependencies = std::map<Task_ptr, Tags>;
        Dependencies dependencies;

		Planner(const std::string &catName = ""): categoryName(catName) {}

		bool add_workers(const gubg::file::File &workers_chaiscript);
        void set_nr_working_days(unsigned int nr) {nr_working_days = nr;}

		ReturnCode run()
		{
			MSS_BEGIN(ReturnCode, logns);

			//Use model to build the gubg::planning::Task tree into root
            parseError_.str("");
			gubg::tree::dfs::iterate(model(), *this);
			if (!parseError_.str().empty())
			{
				std::cout << parseError_.str();
				MSS(ReturnCode::ParsingError);
			}

            //Setup dependencies
            for (const auto &p: dependencies)
            {
                //We will change task, but its operator<() on the Task::Ptr should remain the same
                auto task = p.first;
                S(logns);L("Setting dependencies for " << task->fullName());
                const auto &tags = p.second;
                for (const auto &tag: tags)
                {
                    L(" => " << tag);
                    auto it = task_per_tag.find(tag);
                    MSS(it != task_per_tag.end(), std::cout << "ERROR: Could not find tag " << tag << " which is used as dependency for " << task->name << std::endl);
                    task->addDependency(it->second);
                }
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
                if (n.total() <= 0)
                    return false;

   				if (!root)
				{
					assert(p.empty());
					//We do not put the root name, it is typically not a task and causes only bloat in the output
					current = root = gubg::planning::Task::create("");
				}
				else
					current = current->addChild(n.desc);

                const auto ok = open_(n);
                if (!ok)
                    parseError_ << "ERROR: Something went wrong" << std::endl;

                return true;
            }
        bool open_(Node &n)
        {
            MSS_BEGIN(bool, logns);
            L(STREAM(n.desc, n.total()));

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
            if (n.attributes.count("startdate"))
            {
                gubg::planning::Day startdate(n.attributes["startdate"]);
                if (!startdate.isValid())
                    parseError_ << "Could not parse startdate attribute for node " << n.desc << ": \"" << n.attributes["startdate"] << "\"" << std::endl;
                else
                    current->setStartdate(startdate);
            }
            if (n.attributes.count("mode"))
            {
                using namespace gubg::planning;
                const auto mode_str = n.attributes["mode"];
                if (false) {}
                else if (mode_str == "async") {current->setMode(Mode::Async);}
                else if (mode_str == "sync") {current->setMode(Mode::Sync);}

            }
            if (n.attributes.count("tag"))
            {
                const auto tag = n.attributes["tag"];
                auto it = task_per_tag.find(tag);
                MSS(it == task_per_tag.end(), parseError_ << "Tag " << tag << " already exists for task " << it->second->name << std::endl);
                task_per_tag[tag] = current;
            }
            if (n.attributes.count("depends"))
            {
                const auto tag = n.attributes["depends"];
                L("Task " << current->fullName() << " depends on " << tag);
                dependencies[current].push_back(tag);
            }
            if (n.attributes.count(categoryName) > 0)
            {
                L("Found a category");
                current->setCategory(n.attributes[categoryName]);
            }
            MSS_END();
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
