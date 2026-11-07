#include "lq/verbs.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

using namespace lq;

// Every form is checked against the web app in web_parity_tests.cpp; these pin down the rules by hand.
TEST_CASE("regular verbs follow the rules and irregular ones their own forms", "[verbs]")
{
    CHECK(conjugate("hablar", Tense::Present) == Paradigm{"hablo", "hablas", "habla", "hablamos", "habláis", "hablan"});
    CHECK(conjugate("ir", Tense::Imperfect) == Paradigm{"iba", "ibas", "iba", "íbamos", "ibais", "iban"});
    // The subjunctive comes from the yo form of the present: tengo → tenga.
    CHECK(conjugate("tener", Tense::Subjunctive)[0] == "tenga");
    CHECK(conjugate("comer", Tense::NearFuture)[3] == "vamos a comer");
    CHECK(conjugate("vivir", Tense::Perfect)[1] == "has vivido");
    CHECK(participle("hacer") == "hecho");
}

TEST_CASE("tenses are known by their web ids and unlock with their lesson", "[verbs]")
{
    for (const Tense tense : kTenses) {
        const TenseInfo &info = tenseInfo(tense);
        CAPTURE(info.id);
        CHECK(tenseFromId(info.id) == tense);
        CHECK_FALSE(info.en.empty());
        CHECK(info.unit >= 0);
        CHECK(info.unit < 25);
    }
    CHECK(tenseInfo(Tense::Present).unit == 0);
    CHECK(tenseInfo(Tense::Preterite).unit > 0);
    CHECK_FALSE(tenseFromId("nope"));
}

TEST_CASE("the eight 80/20 verbs are the core", "[verbs]")
{
    const std::vector<std::string_view> core = coreVerbs();
    CHECK(core.size() == 8);
    CHECK(std::find(core.begin(), core.end(), "tener") != core.end());
    REQUIRE(findVerb("tener") != nullptr);
    CHECK(findVerb("tener")->core);
    CHECK(findVerb("hablar") != nullptr);
    CHECK_FALSE(findVerb("hablar")->core);
    CHECK(findVerb("volar") == nullptr);
    CHECK(std::find(pronouns(2).begin(), pronouns(2).end(), "usted") != pronouns(2).end());
}
