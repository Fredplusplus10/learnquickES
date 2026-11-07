#include "lq/achievements.h"
#include "test_support.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <set>

using namespace lq;
using lq::test::d;

namespace {

std::vector<std::string_view> ids(const std::vector<Achievement> &list)
{
    std::vector<std::string_view> out;
    for (const Achievement &a : list)
        out.push_back(a.id);
    return out;
}

} // namespace

TEST_CASE("the same 16 achievements as the web app, each with an icon the app draws", "[achievements]")
{
    const auto &list = achievementList();
    CHECK(list.size() == 16);
    std::set<std::string_view> unique;
    for (const Achievement &a : list) {
        unique.insert(a.id);
        CHECK_FALSE(a.title.empty());
        CHECK_FALSE(a.how.empty());
        CHECK_FALSE(a.icon.empty());
    }
    CHECK(unique.size() == list.size());
}

TEST_CASE("achievements are earned from what the learner did, once, on the day they did it", "[achievements]")
{
    LearnerState state = defaultState(d("2026-11-02"));
    CHECK(checkAchievements(state, d("2026-11-02")).empty());

    learnUnit(state, 0, {"a"}, 1.0, d("2026-11-02"));
    logExchange(state, 30, "", "", d("2026-11-02"));
    CHECK(ids(checkAchievements(state, d("2026-11-02"))) == std::vector<std::string_view>{"first_lesson", "exchange_1"});
    CHECK(checkAchievements(state, d("2026-11-03")).empty());  // never twice
    CHECK(state.achievements.at("first_lesson") == d("2026-11-02"));

    for (const char *id : {"phone", "mac", "social", "youtube", "siri"})
        setMission(state, id, true, d("2026-11-04"));
    logInput(state, 600, "podcast", "", d("2026-11-04"));
    logInput(state, 300, "island", "", d("2026-11-04"));
    saveCheckpoint(state, 5, 0.72, d("2026-11-04"));
    CHECK(ids(checkAchievements(state, d("2026-11-04")))
          == std::vector<std::string_view>{"input_10h", "island_5h", "devices", "b1_mock"});

    // Unticking a mission later does not take the badge away.
    setMission(state, "phone", false, d("2026-11-05"));
    CHECK(checkAchievements(state, d("2026-11-05")).empty());
    CHECK(state.achievements.contains("devices"));
}

TEST_CASE("the preterite badge needs both enough answers and 85 % of them right", "[achievements]")
{
    LearnerState state = defaultState(d("2026-11-02"));
    for (int i = 0; i < 29; ++i)
        recordAnswer(state, "past", true, false, false, d("2026-11-02"));
    CHECK(checkAchievements(state, d("2026-11-02")).empty());  // 29 answers
    recordAnswer(state, "past", true, false, false, d("2026-11-02"));
    for (int i = 0; i < 6; ++i)
        recordAnswer(state, "past", false, false, false, d("2026-11-02"));
    CHECK(checkAchievements(state, d("2026-11-02")).empty());  // 30 of 36 is 83 %
    recordAnswer(state, "past", true, false, false, d("2026-11-02"));
    recordAnswer(state, "past", true, false, false, d("2026-11-02"));
    recordAnswer(state, "past", true, false, false, d("2026-11-02"));
    recordAnswer(state, "past", true, false, false, d("2026-11-02"));
    CHECK(ids(checkAchievements(state, d("2026-11-02"))) == std::vector<std::string_view>{"past_master"});  // 34 of 40
}

TEST_CASE("Cognate Hunter counts cognates and false friends together", "[achievements]")
{
    // False friends have their own skill now; answers from before the split stayed under "cognates".
    LearnerState state = defaultState(d("2026-11-02"));
    for (int i = 0; i < 30; ++i)
        recordAnswer(state, "cognates", true, false, false, d("2026-11-02"));
    for (int i = 0; i < 19; ++i)
        recordAnswer(state, "falsefriends", true, false, false, d("2026-11-02"));
    CHECK(checkAchievements(state, d("2026-11-02")).empty());  // 49 right
    recordAnswer(state, "falsefriends", true, false, false, d("2026-11-02"));
    CHECK(ids(checkAchievements(state, d("2026-11-02"))) == std::vector<std::string_view>{"cognates_50"});
}

TEST_CASE("B1 readiness signals show the learner's own numbers", "[achievements]")
{
    LearnerState state = defaultState(d("2026-11-02"));
    learnUnit(state, 0, {"a", "b"}, 1.0, d("2026-11-02"));
    logInput(state, 90, "series", "", d("2026-11-02"));
    const std::vector<ReadinessSignal> signals = readiness(state, totals(state, d("2026-11-02")));
    REQUIRE(signals.size() == 7);
    CHECK(signals[0].text == "All 25 lessons (1/25)");
    CHECK_FALSE(signals[0].met);
    CHECK(signals[1].text == "Fui vs. iba: 85%+ over 30+ answers (–)");
    CHECK(signals[4].text == "50+ hours of listening (1.5 h)");
    CHECK(std::none_of(signals.begin(), signals.end(), [](const ReadinessSignal &s) { return s.met; }));
}
