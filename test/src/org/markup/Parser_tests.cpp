#include <org/markup/Parser.hpp>

#include <catch.hpp>

using namespace org;

TEST_CASE("markup::Parser.pop_prefix tests", "[org][markup][Parser][pop_prefix]")
{
    struct Scn
    {
        markup::Type markup_type = markup::Type::None;
        std::string line;
    };
    Scn scn;

    struct Exp
    {
        bool ok = true;
        std::string prefix;
        bool is_bullet = false;
        std::string rest;
    };
    Exp exp;

    SECTION("Markdown")
    {
        scn.markup_type = markup::Type::Markdown;
        SECTION("empty") {}
        SECTION("is_bullet")
        {
            exp.is_bullet = true;
            exp.prefix = "* ";
            exp.rest = "rest";
            scn.line = "* rest";
        }
        SECTION("!is_bullet")
        {
            exp.is_bullet = false;
            exp.prefix = "# ";
            exp.rest = "rest";
            scn.line = "# rest";
        }
    }

    SECTION("Org")
    {
        scn.markup_type = markup::Type::Org;
        SECTION("empty") {}
    }

    markup::Parser parser{scn.markup_type};

    gubg::Strange prefix_se;
    bool is_bullet = false;
    gubg::Strange line_se{scn.line};
    const auto ok = parser.pop_prefix(prefix_se, is_bullet, line_se);
    REQUIRE(ok == exp.ok);
    if (ok)
    {
        REQUIRE(prefix_se.str() == exp.prefix);
        REQUIRE(is_bullet == exp.is_bullet);
        REQUIRE(line_se.str() == exp.rest);
    }
}

// TEST_CASE("markup::Parser.pop_prefix tests", "[org][markup][Parser][pop_prefix]")
// {
//     struct Scn
//     {
//     };
//     Scn scn;

//     struct Exp
//     {
//     };
//     Exp exp;
// }
