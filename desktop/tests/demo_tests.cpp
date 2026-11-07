#include "lq/coach.h"
#include "lq/demo.h"
#include "test_support.h"

#include <catch2/catch_test_macros.hpp>

using namespace lq;
using lq::test::d;

TEST_CASE("the seeded generator matches the web app's sequence", "[demo]")
{
    // First draws of LQ.util.seeded('demo') in Node, as exact multiples of 2^-32.
    SeededRandom rnd("demo");
    CHECK(rnd.next() == 3603001048.0 / 4294967296.0);
    CHECK(rnd.next() == 581339189.0 / 4294967296.0);
    CHECK(rnd.next() == 238296426.0 / 4294967296.0);
    SeededRandom other("other");
    const double first = SeededRandom("demo").next();
    CHECK(other.next() != first);
}

TEST_CASE("the demo learner is internally consistent", "[demo]")
{
    const Date today = d("2026-11-02");
    const LearnerState state = demoState(today, lq::test::course().unitWordIds());
    CHECK(state.done.size() > 5);
    CHECK(state.units.size() >= 3);
    CHECK(state.frozen.size() == 1);
    // The streak survives thanks to the freeze.
    CHECK(streakInfo(state, today).count >= 5);
    CHECK(countStudyDays(state.plan.start, addDays(today, -1), state.plan) == 16);
    // Every finished lesson brought its words into the deck, and every card belongs to a finished lesson.
    CHECK_FALSE(state.cards.empty());
    for (const auto &[id, card] : state.cards)
        CHECK(state.units.contains(card.unit));
    // Two reflections, so the coach screen has a taste to show; the next one is not due yet.
    CHECK(state.reflections.size() == 2);
    CHECK_FALSE(reflectionDue(state));
    CHECK(taste(state).favourites() == std::vector<std::string>{"listen", "speak"});
}

TEST_CASE("without the course the demo learner still works, with an empty deck", "[demo]")
{
    const LearnerState state = demoState(d("2026-11-02"), {});
    CHECK(state.cards.empty());
    CHECK(state.units.size() >= 3);
}

TEST_CASE("the demo learner logs its listening, devices and a conversation, and has earned what they give", "[demo]")
{
    const lq::Date today = lq::test::d("2026-11-23");
    const lq::LearnerState state = lq::demoState(today, lq::test::course().unitWordIds());
    int logged = 0;
    for (const lq::InputEntry &entry : state.input)
        logged += entry.minutes;
    int listened = 0;
    for (const auto &[day, activity] : state.activity)
        listened += activity.inputMinutes;
    CHECK(logged == listened);  // every minute of listening is in the log
    CHECK(state.missions.size() == 3);
    REQUIRE(state.exchanges.size() == 1);
    CHECK(state.achievements.contains("first_lesson"));
    CHECK(state.achievements.contains("exchange_1"));
    CHECK(state.achievements.contains("input_10h"));
}
