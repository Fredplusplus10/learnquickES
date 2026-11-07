#include "lq/text.h"

#include <catch2/catch_test_macros.hpp>

using namespace lq;

TEST_CASE("Spanish letters are lowercased and stripped of accents", "[text]")
{
    CHECK(toLower("ÁRBOL Ñandú ÜBER") == "árbol ñandú über");
    CHECK(stripAccents("árbol ñandú über") == "arbol nandu uber");
    // A decomposed accent (e + U+0301) is dropped too.
    CHECK(stripAccents("cafe\xCC\x81") == "cafe");
    CHECK(codePointCount("ñandú") == 5);
    CHECK(codePointCount("") == 0);
}

TEST_CASE("text splits into normalized words", "[text]")
{
    CHECK(words("  ¿Hola,  qué TAL?  ") == std::vector<std::string>{"hola", "qué", "tal"});
    CHECK(words("").empty());
    CHECK(words("¡!").empty());
}

TEST_CASE("any accepted answer matches; the matched one is reported", "[text]")
{
    const AnswerCheck second = checkAnswer("Ella es", {"él es", "ella es"});
    CHECK(second.ok);
    CHECK_FALSE(second.almost);
    CHECK(second.expected == "ella es");

    const AnswerCheck accents = checkAnswer("tambien", {"también"});
    CHECK(accents.ok);
    CHECK(accents.almost);
    CHECK(accents.expected == "también");

    const AnswerCheck wrong = checkAnswer("adiós", {"hola", "buenas"});
    CHECK_FALSE(wrong.ok);
    CHECK(wrong.expected == "hola");

    CHECK(checkAnswer("  ¿?  ", {"hola"}).empty);
    CHECK_FALSE(checkAnswer("hola", {}).ok);
}

TEST_CASE("broken UTF-8 never crashes the checker", "[text]")
{
    CHECK_FALSE(checkAnswer("\xC3", {"á"}).ok);
    CHECK_FALSE(checkAnswer("ab\xE2\x82", {"ab"}).empty);
    CHECK(normalize("\xFF\xFE") != "");
}
