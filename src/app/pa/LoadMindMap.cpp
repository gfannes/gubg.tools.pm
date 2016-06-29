#include "pa/LoadMindMap.hpp"
#include "pa/Model.hpp"
#include "gubg/parse/xml/Parser.hpp"
#include "gubg/parse/tree/Parser.hpp"
#include "gubg/parse/basic.hpp"
#include "gubg/file/Filesystem.hpp"
#include "gubg/tree/dfs/Iterate.hpp"
#include <string>
#include <fstream>
#include <map>
using namespace pa;
using namespace gubg::file;
using namespace std;

namespace pa
{
    const char *logns = "LoadMindMap";

    using Location = std::vector<Node*>;

    struct TreeParser: gubg::parse::tree::Parser_crtp<TreeParser>
    {
        Node &root;
        const string value;
        const string fraction;
        const double default_fraction;
        const double value2days;
        Location location;
        TreeParser(Node &r, string n, string f, double df, double value2days):root(r), value(n), fraction(f), default_fraction(df), value2days(value2days){}

        template <typename Name, typename Attr>
            void parser_open(const Name &name, const Attr &attr)
            {
                S("TreeParser");
                L(C(name));
                if (location.empty())
                    location.push_back(&root);
                else
                {
                    location.back()->childs.push_back(Node());
                    location.push_back(&location.back()->childs.back());
                }
                location.back()->desc = name;
                location.back()->fraction = default_fraction;
                for (const auto &p: attr)
                {
                    const auto &k = p.first;
                    const auto &v = p.second;
                    if (false) {}
                    else if (k == value)
                    {
                        const auto totals = std::stod(v);
                        location.back()->value = totals*value2days;
                    }
                    else
                    {
                        location.back()->attributes.emplace(k, v);
                    }
                }
            }
            void parser_close()
            {
                S("TreeParser");
                location.pop_back();
            }
    };

    struct XmlParser: gubg::xml::Parser_crtp<XmlParser>
    {
        Node &root;
        const string value;
        const string fraction;
        const double default_fraction;
        const double value2days;
        Location location;
        XmlParser(Node &r, string n, string f, double df, double value2days):root(r), value(n), fraction(f), default_fraction(df), value2days(value2days){}

		typedef gubg::xml::Path Path;
		typedef gubg::xml::Attributes Attributes;
        void parser_open(const string &tag, const Path &path)
        {
            if (tag == "node")
            {
                S(logns);L(tag);
                if (location.empty())
                    location.push_back(&root);
                else
                {
                    location.back()->childs.push_back(Node());
                    location.push_back(&location.back()->childs.back());
                }
                location.back()->fraction = default_fraction;
            }
        }
        void parser_close(const string &tag, const Path &path)
        {
            if (tag == "node")
            {
                S(logns);L(tag);
                location.pop_back();
            }
        }
        pa::ReturnCode parser_attr(const Attributes &attrs, const Path &path)
		{
			MSS_BEGIN(pa::ReturnCode);
			if (path.back() == "node")
			{
				auto it = attrs.find("TEXT");
				auto id = attrs.find("ID");
				MSS(it != attrs.end());
				L(it->second);
				location.back()->desc = it->second;
                if (id != attrs.end())
                    location.back()->id = id->second;
			}
			if (path.back() == "attribute")
			{
				auto n = attrs.find("NAME");
				auto v = attrs.find("VALUE");
				MSS(n != attrs.end() && v != attrs.end());
				if (n->second == value)
				{
					//Totals value
					MSS(!location.empty());
					auto o = attrs.find("OBJECT");
                    double totals = 0.0;
					if (o != attrs.end())
					{
						gubg::Strange strange(o->second);
						MSS(strange.pop_if("org.freeplane.features.format.FormattedNumber|"));
						MSS(strange.popFloat(totals));
					}
					else
					{
						gubg::Strange strange(v->second);
						MSS(strange.popFloat(totals));
					}
                    location.back()->value = totals*value2days;
					L("Detected value " << value << " for " << location.back()->desc << ": " << location.back()->value);
				}
				else if (n->second == fraction)
				{
					//Fraction
					MSS(!location.empty());
					auto o = attrs.find("OBJECT");
					if (o != attrs.end())
					{
						gubg::Strange strange(o->second);
						MSS(strange.pop_if("org.freeplane.features.format.FormattedNumber|"));
						MSS(strange.popFloat(location.back()->fraction));
					}
					else
					{
						gubg::Strange strange(v->second);
						MSS(strange.popFloat(location.back()->fraction));
					}
					L("Detected fraction " << fraction << " for " << location.back()->desc << ": " << location.back()->fraction);
				}
				else
				{
					//General attribute
					MSS(!location.empty());
					location.back()->attributes.emplace(n->second, v->second);
					L("Detected something else " << n->second << " for " << location.back()->desc << ": " << v->second);
				}
			}
			MSS_END();
		}
    };
}
namespace 
{
	struct Pruner
	{
		typedef deque<string> AllowedPath;
		typedef list<AllowedPath> AllowedPaths;
		AllowedPaths allowedPaths;


		void add(const AllowedPath &allowedPath)
		{
			allowedPaths.push_back(allowedPath);
		}

		struct Compare
		{
			bool operator()(const string &desc, const Node *node) const
			{
				return desc.empty() || node->desc == desc;
			}
		};
		template <typename Path>
			bool open(Node &n, Path &p) const
			{
                S(logns);L(n.desc);
				Path pp(p);
				pp.push_back(&n);
				for (const auto &ap: allowedPaths)
				{
					auto ap_beg = ap.cbegin(), ap_end = ap.cend();
					auto pp_beg = pp.cbegin(), pp_end = pp.cend();
					if (pp.size() <= ap.size())
						ap_end = ap_beg + pp.size();
					else
						pp_end = pp_beg + ap.size();
					if (std::equal(ap_beg, ap_end, pp_beg, Compare()))
						return true;
				}
                L("Pruning this branch");
				n.childs.clear();
				n.fraction = 0;
				return false;
			}
		template <typename Path>
			void close(Node &n, Path &p) const { }
	};
    struct Aggregate
    {
        template <typename Path>
            bool open(Node &n, Path &p) const
            {
				n.cumul = n.value;
                return true;
            }
        template <typename Path>
            void close(Node &n, Path &p) const
            {
                //We add the cumul of n to the parent, if any
                if (!p.empty())
                    p.back()->cumul += n.total();
            }
    };
    struct Distribute
    {
		const std::string attr_;
		Distribute(std::string attr): attr_(attr){}
        template <typename Path>
            bool open(Node &n, Path &p) const
            {
				if (p.empty())
					return true;
				const auto &attrs = p.back()->attributes;
                auto range = attrs.equal_range(attr_);
                for (auto it = range.first; it != range.second; ++it)
                    n.attributes.insert(*it);
				return true;
            }
        template <typename Path>
            void close(Node &n, Path &p) const
            {
            }
    };
}
pa::ReturnCode LoadMindMap::execute(const Options &options)
{
	MSS_BEGIN(ReturnCode, logns);
    L("Loading mindmap: " << STREAM(options.input, options.value, options.fraction, options.fraction_default));

	double default_fraction = 1.0;
	if (!options.fraction_default.empty())
	{
		gubg::Strange strange(options.fraction_default);
		MSS(strange.popFloat(default_fraction));
	}

    string content;
    MSS(read(content, options.input));

    const auto ext = options.input.extension();
    if (false) {}
    else if (ext == "mm")
    {
        XmlParser p(model(), options.value, options.fraction, default_fraction, options.value2days);
        MSS(p.process(content));
    }
    else if (ext == "tree")
    {
        TreeParser p(model(), options.value, options.fraction, default_fraction, options.value2days);
        L("Before process");
        MSS(p.process(content));
        L("After process");
    }
    else
    {
        MSS(pa::ReturnCode::UnknownExtension, cerr << "Unknown extension " << ext << endl);
    }
    L("Loading the model went OK");
    L(STREAM(model().total()));

	{
		Pruner pruner;
		for (auto line: options.lines)
			pruner.add(gubg::parse::tokenize(line, "/"));
		gubg::tree::dfs::iterate(model(), pruner);
        L(STREAM(model().total()));
	}
	gubg::tree::dfs::iterate(model(), Aggregate());
    L(STREAM(model().total()));
	if (!options.category.empty())
		gubg::tree::dfs::iterate(model(), Distribute(options.category));
    L(STREAM(model().total()));

	MSS_END();
}
