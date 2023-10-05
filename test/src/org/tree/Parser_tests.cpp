#include <org/tree/Parser.hpp>

#include <catch.hpp>

using namespace org;

TEST_CASE("XXX tests", "[ut][org][tree][Parser]")
{
    struct Scn
    {
        std::string content;
    };
    Scn scn;

    struct Exp
    {
        bool ok = true;
    };
    Exp exp;

    SECTION("3 line")
    {
        scn.content = "\n# title0\n## title0.0\n## title0.1";
    }

    tree::Parser parser;
    REQUIRE(parser.init(markup::Type::Markdown));
    const auto ok = parser.parse(std::move(scn.content));
    REQUIRE(ok == exp.ok);
    if (ok)
    {
        std::cout << parser.root << std::endl;
    }
}
