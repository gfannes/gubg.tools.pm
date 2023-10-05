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
            SECTION("* ")
            {
                exp.prefix = "* ";
                exp.rest = "rest";
                scn.line = "* rest";
            }
            SECTION(" * ")
            {
                exp.prefix = " * ";
                exp.rest = "rest";
                scn.line = " * rest";
            }
            SECTION(" - ")
            {
                exp.prefix = " - ";
                exp.rest = "rest";
                scn.line = " - rest";
            }
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
        SECTION("is_bullet")
        {
            exp.is_bullet = true;
            SECTION("- ")
            {
                exp.prefix = "- ";
                exp.rest = "rest";
                scn.line = "- rest";
            }
            SECTION(" - ")
            {
                exp.prefix = " - ";
                exp.rest = "rest";
                scn.line = " - rest";
            }
        }
        SECTION("!is_bullet")
        {
            exp.is_bullet = false;
            exp.prefix = "* ";
            exp.rest = "rest";
            scn.line = "* rest";
        }
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

TEST_CASE("markup::Parser.extract_link tests", "[org][markup][Parser][extract_link]")
{
    struct Scn
    {
        markup::Type markup_type = markup::Type::None;
        std::string line;
    };
    Scn scn;

    struct Exp
    {
        bool ok = false;
        std::string text;
        std::string link;
    };
    Exp exp;

    SECTION("Markdown")
    {
        scn.markup_type = markup::Type::Markdown;
        SECTION("no link")
        {
            SECTION("default") {}
            SECTION("bla") { scn.line = "bla"; }
        }
        SECTION("link")
        {
            exp.ok = true;
            exp.text = "text";
            exp.link = "link";
            scn.line = "bla [text](link) bli";
        }
    }
    SECTION("Org")
    {
        scn.markup_type = markup::Type::Org;
        SECTION("no link")
        {
            SECTION("default") {}
            SECTION("bla") { scn.line = "bla"; }
        }
        SECTION("link")
        {
            exp.ok = true;
            exp.text = "text";
            exp.link = "link";
            scn.line = "bla [[link][text]] bli";
        }
    }

    markup::Parser parser{scn.markup_type};

    gubg::Strange text_se, link_se, line_se{scn.line};
    const auto ok = parser.extract_link(text_se, link_se, line_se);
    REQUIRE(ok == exp.ok);
    if (ok)
    {
        REQUIRE(text_se.str() == exp.text);
        REQUIRE(link_se.str() == exp.link);
    }
}
