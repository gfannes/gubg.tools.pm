#include "pa/ShowWBS.hpp"
#include "pa/Model.hpp"
#include "gubg/tree/dfs/Iterate.hpp"
#include "gubg/planning/Day.hpp"
#include "gubg/xml/Builder.hpp"
#include <fstream>
using namespace pa;
using namespace std;

namespace 
{
    struct Show
    {
        ostream &os;
        Show(ostream &o):os(o){}
        template <typename Path>
            bool open(Node &n, Path &p) const
            {
                if (n.total() <= 0)
                    return false;
                os << string(p.size()*2, ' ') << n.desc << " (" << n.total() << ": " << n.fraction << " * " << n.cumul << ")" << endl;
                return true;
            }
        template <typename Path>
            void close(Node &n, Path &p) const
            {
            }
    };
    struct ShowHtml
    {
        ostream &os;
        ShowHtml(ostream &o):os(o){}
        template <typename Path>
            bool open(Node &n, Path &p) const
            {
                if (n.total() <= 0)
                    return false;
                os << "<ul>";
                os << "<li>" << n.desc << " (" << n.total() << ": " << n.fraction << " * " << n.cumul << ")" << "</li>";
                return true;
            }
        template <typename Path>
            void close(Node &n, Path &p) const
            {
                os << "</ul>";
            }
    };
}
pa::ReturnCode ShowWBS::execute(const Options &options)
{
	MSS_BEGIN(ReturnCode);L("Showing WBS");

	gubg::tree::dfs::iterate(model(), ShowHtml(cout));

	if (!options.output.name().empty())
	{
        {
            ofstream fo(options.output.name());
            fo << "# Work breakdown on " << gubg::planning::Day::today() << endl << endl;
            fo << "Units are days" << endl << endl;
            gubg::tree::dfs::iterate(model(), Show(fo));
        }
        {
            using namespace gubg::xml::builder;
            auto fn = options.output;
            fn.setExtension("html");
            ofstream fo(fn.name());
            Tag html(fo, "html");
            auto body = html.tag("body");
            body.tag("h1") << "Work breakdown on " << gubg::planning::Day::today();
            body.tag("p").tag("i") << "Units are days";
            gubg::tree::dfs::iterate(model(), ShowHtml(fo));
        }
	}

	MSS_END();
}
