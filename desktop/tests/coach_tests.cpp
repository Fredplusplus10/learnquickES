// The coach's suggestions, and the reflections that teach it what the learner enjoys.
#include "lq/coach.h"
#include "lq/demo.h"
#include "test_support.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>

using namespace lq;
using lq::test::d;

namespace {

const Date kToday = d("2026-11-02");  // a Monday

LearnerState learner(Date start = kToday)
{
    LearnerState state = defaultState(start);
    state.plan.days = 30;
    return state;
}

std::vector<std::string> ids(const std::vector<CoachTip> &tips)
{
    std::vector<std::string> out;
    for (const CoachTip &tip : tips)
        out.push_back(tip.id);
    return out;
}

// The tip a rule produced, or nullptr. Copied, so it outlives the list.
std::optional<CoachTip> tipOf(const std::vector<CoachTip> &tips, std::string_view id)
{
    const auto it = std::find_if(tips.begin(), tips.end(), [id](const CoachTip &tip) { return tip.id == id; });
    return it == tips.end() ? std::nullopt : std::optional<CoachTip>(*it);
}

std::optional<CoachTip> tipOf(const LearnerState &state, std::string_view id, Date today = kToday, std::string_view after = {})
{
    return tipOf(coachTips(state, test::course(), today, after), id);
}

Reflection reflection(std::vector<std::string> offered, std::vector<std::string> liked,
                      std::optional<Feel> feel = Feel::Right, Date date = kToday)
{
    return {date, "mixed", std::move(offered), std::move(liked), feel, ""};
}

std::vector<Item> itemsOf(ItemType type, int n)
{
    std::vector<Item> items(static_cast<std::size_t>(n));
    for (Item &item : items)
        item.type = type;
    return items;
}

// Draws 10 of 40 listening and 40 multiple-choice items, 50 times; returns how many of each came out.
std::pair<int, int> listenVersusChoice(const Taste &taste)
{
    std::vector<Item> pool = itemsOf(ItemType::Listen, 40);
    const std::vector<Item> choices = itemsOf(ItemType::Choice, 40);
    pool.insert(pool.end(), choices.begin(), choices.end());
    int listen = 0;
    int choice = 0;
    for (std::uint32_t seed = 1; seed <= 50; ++seed) {
        SeededRandom rnd(seed);
        for (const Item &item : preferredItems(pool, 10, taste, rnd))
            ++(item.type == ItemType::Listen ? listen : choice);
    }
    return {listen, choice};
}

} // namespace

TEST_CASE("the reflection comes every three sessions, and \"not now\" puts it off as long", "[coach]")
{
    LearnerState state = learner();
    countSession(state);
    countSession(state);
    CHECK_FALSE(reflectionDue(state));
    countSession(state);
    CHECK(reflectionDue(state));

    postponeReflection(state);
    CHECK_FALSE(reflectionDue(state));
    for (int i = 0; i < kReflectEvery; ++i)
        countSession(state);
    REQUIRE(reflectionDue(state));

    addReflection(state, reflection({"listen"}, {"listen"}));
    CHECK_FALSE(reflectionDue(state));
    CHECK(state.reflections.size() == 1);
}

TEST_CASE("a long note is cut after 280 characters, never inside one", "[coach]")
{
    LearnerState state = learner();
    Reflection r = reflection({}, {});
    for (int i = 0; i < 300; ++i)
        r.note += "é";  // two bytes in UTF-8
    addReflection(state, r);
    CHECK(state.reflections.back().note.size() == 2 * kMaxNoteLength);
}

TEST_CASE("exercise kinds have the names the screens use", "[coach]")
{
    CHECK(itemKind(ItemType::Conjugate) == "conj");
    CHECK(itemKind(ItemType::Listen) == "listen");
    CHECK(kindLabel("order") == "building sentences");
    CHECK(kindLabel("unknown") == "unknown");
}

TEST_CASE("taste: kinds enjoyed weigh more, kinds passed over weigh less, the rest stay neutral", "[coach]")
{
    LearnerState state = learner();
    CHECK(taste(state).reflections == 0);
    CHECK(taste(state).weight("listen") == 1.0);

    addReflection(state, reflection({"listen", "choice"}, {"listen"}));
    Taste once = taste(state);
    CHECK(once.weight("listen") > kFavourite);
    CHECK(once.weight("choice") < 1.0);
    CHECK(once.weight("match") == 1.0);  // never offered: no opinion yet
    CHECK(once.favourites() == std::vector<std::string>{"listen"});

    // Liked again and again weighs more than liked once, and a kind passed over is never ruled out.
    for (int i = 0; i < 3; ++i)
        addReflection(state, reflection({"listen", "choice"}, {"listen"}));
    const Taste steady = taste(state);
    CHECK(steady.weight("listen") > once.weight("listen"));
    CHECK(steady.weight("listen") < 1.5);
    CHECK(steady.weight("choice") > 0.5);

    // Only the latest reflections count, so taste can change.
    for (std::size_t i = 0; i < kTasteMemory; ++i)
        addReflection(state, reflection({"listen", "choice"}, {"choice"}));
    const Taste changed = taste(state);
    CHECK(changed.reflections == static_cast<int>(kTasteMemory));
    CHECK(changed.weight("listen") < 1.0);
    CHECK(changed.favourites() == std::vector<std::string>{"choice"});
}

TEST_CASE("taste keeps how the last asked session felt, and says what it changes", "[coach]")
{
    LearnerState state = learner();
    CHECK(tasteSummary(taste(state)).starts_with("Noted."));

    addReflection(state, reflection({"listen", "order", "choice"}, {"listen", "order"}, Feel::Hard));
    // Reflecting on the coach screen asks no feel, so the last one stays.
    addReflection(state, Reflection{kToday, "coach", {"conj"}, {}, std::nullopt, ""});
    const Taste t = taste(state);
    REQUIRE(t.feel == Feel::Hard);
    CHECK(tasteSummary(t) == "Mixed practice now leans towards listening and building sentences. After a hard session it also "
                             "gives you more answers to choose from, until it feels right.");
}

TEST_CASE("mixed practice leans towards what the learner enjoys without ruling anything out", "[coach]")
{
    const auto [neutralListen, neutralChoice] = listenVersusChoice(Taste{});
    CHECK(neutralListen + neutralChoice == 500);

    LearnerState state = learner();
    for (int i = 0; i < 6; ++i)
        addReflection(state, reflection({"listen", "choice"}, {"listen"}));
    const auto [listen, choice] = listenVersusChoice(taste(state));
    CHECK(listen > neutralListen);
    CHECK(listen * 2 > choice * 3);  // weights of about 1.4 against 0.6
    CHECK(choice > 0);

    // After an easy session, building and typing weigh more than choosing, even before any taste.
    LearnerState easy = learner();
    addReflection(easy, reflection({}, {}, Feel::Easy));
    const auto [built, chosen] = listenVersusChoice(taste(easy));
    CHECK(built * 10 > chosen * 13);  // 1.3 against 0.7

    SeededRandom rnd(3);
    CHECK(preferredItems(itemsOf(ItemType::Match, 4), 10, taste(state), rnd).size() == 4);
}

TEST_CASE("on day 1 the coach points at today's plan", "[coach]")
{
    LearnerState state = learner();
    syncReview(state, kToday);  // an empty deck has nothing to review, as when the app opens
    const std::vector<CoachTip> tips = coachTips(state, test::course(), kToday);
    REQUIRE(ids(tips) == std::vector<std::string>{"plan"});
    CHECK(tips[0].title.starts_with("Next on today's plan: Lesson 1, "));
    CHECK(tips[0].task == Task::Lesson);
    CHECK(tips[0].detail.find("tasks left today") != std::string::npos);
    // Right after the lesson, the coach moves on to the next task.
    CHECK(tipOf(state, "plan", kToday, "lesson") == std::nullopt);
}

TEST_CASE("a pile of due cards comes first, but not right after a review", "[coach]")
{
    LearnerState state = learner();
    for (int i = 0; i < 20; ++i) {
        Card card = newCard(d("2026-11-01"));  // a new card is due the next day
        card.seq = i;
        state.cards.emplace("word" + std::to_string(i), card);
    }
    const std::vector<CoachTip> tips = coachTips(state, test::course(), kToday);
    REQUIRE_FALSE(tips.empty());
    CHECK(tips[0].id == "review");
    CHECK(tips[0].trainer == "review");
    CHECK(tips[0].detail.starts_with("20 cards are due."));
    CHECK(tipOf(state, "review", kToday, "review") == std::nullopt);
}

TEST_CASE("the plan's next task is not suggested twice", "[coach]")
{
    // The sample learner's next task is the review, and 37 cards are due: one suggestion leads there, not two.
    const LearnerState state = demoState(kToday, test::course().unitWordIds());
    const std::vector<CoachTip> tips = coachTips(state, test::course(), kToday);
    REQUIRE_FALSE(tips.empty());
    CHECK(tips[0].id == "review");
    CHECK(tipOf(state, "plan") == std::nullopt);
}

TEST_CASE("behind the plan, the coach says by how much and asks for no heroics", "[coach]")
{
    // Two weeks of weekdays since the start, nothing done.
    const auto behind = tipOf(learner(d("2026-10-19")), "behind");
    REQUIRE(behind);
    CHECK(behind->detail.starts_with("You are 10 days behind."));
}

TEST_CASE("the weakest skill of the last two weeks gets a suggestion, with its numbers", "[coach]")
{
    LearnerState state = learner();
    state.activity[d("2026-10-30")].answers["verbs"] = {4, 10};
    state.activity[d("2026-10-30")].answers["vocab"] = {9, 10};
    state.activity[d("2026-10-01")].answers["gender"] = {0, 20};  // too long ago
    state.activity[kToday].answers["past"] = {1, 5};             // too few answers to judge
    const auto skill = tipOf(state, "skill");
    REQUIRE(skill);
    CHECK(skill->trainer == "verbs");
    CHECK(skill->title == "Give verb endings some attention");
    CHECK(skill->detail.starts_with("40% right over the last two weeks (4 of 10)."));
    CHECK(tipOf(state, "skill", kToday, "verbs") == std::nullopt);
}

TEST_CASE("false friends are judged apart from cognates, each pointing to its own trainer", "[coach]")
{
    LearnerState state = learner();
    state.activity[d("2026-10-30")].answers["cognates"] = {9, 10};
    state.activity[d("2026-10-30")].answers["falsefriends"] = {3, 10};
    auto skill = tipOf(state, "skill");
    REQUIRE(skill);
    CHECK(skill->trainer == "falsefriends");
    CHECK(skill->title == "Give false friends some attention");

    state.activity[d("2026-10-30")].answers["cognates"] = {2, 10};
    state.activity[d("2026-10-30")].answers["falsefriends"] = {9, 10};
    skill = tipOf(state, "skill");
    REQUIRE(skill);
    CHECK(skill->trainer == "cognates");
    CHECK(skill->title == "Give cognates some attention");
}

TEST_CASE("a lesson with a low best score is worth another pass, but not right after a lesson", "[coach]")
{
    LearnerState state = learner();
    state.units[0] = {kToday, 0.9, 0.9, 1};
    state.units[3] = {kToday, 0.5, 0.5, 1};
    const auto lesson = tipOf(state, "lesson");
    REQUIRE(lesson);
    CHECK(lesson->unit == 3);
    CHECK(lesson->title == "Practise Lesson 4 again");
    CHECK(tipOf(state, "lesson", kToday, "lesson") == std::nullopt);
}

TEST_CASE("days without speaking out loud bring shadowing up", "[coach]")
{
    LearnerState state = learner(d("2026-10-19"));
    state.units[0] = {d("2026-10-19"), 0.9, 0.9, 1};
    state.activity[d("2026-10-28")].speak = 3;
    const auto speak = tipOf(state, "speak");
    REQUIRE(speak);
    CHECK(speak->trainer == "speak");
    CHECK(speak->detail.starts_with("Nothing said out loud for 5 days."));
    CHECK(tipOf(state, "speak", kToday, "speak") == std::nullopt);

    state.activity[d("2026-10-31")].speak = 1;
    CHECK(tipOf(state, "speak") == std::nullopt);
}

TEST_CASE("a checkpoint in the next few plan days sends the learner to mixed practice", "[coach]")
{
    LearnerState state = learner(d("2026-10-01"));
    const std::vector<ScheduleEntry> &plan = schedule(30);
    const auto checkpoint = std::find_if(plan.begin(), plan.end(), [](const ScheduleEntry &e) { return e.type == DayType::Checkpoint; });
    REQUIRE(checkpoint != plan.end());
    for (int n = 1; n <= checkpoint->n - 3; ++n)
        state.done.push_back({n, d("2026-10-30")});
    state.units[0] = {d("2026-10-01"), 0.9, 0.9, 1};
    const auto tip = tipOf(state, "checkpoint");
    REQUIRE(tip);
    CHECK(tip->title == "The phase 1 checkpoint is in 2 plan days");
    CHECK(tip->trainer == "mixed");
}

TEST_CASE("a hard or easy session shapes the suggestions for a week", "[coach]")
{
    LearnerState state = learner();
    addReflection(state, reflection({"choice"}, {}, Feel::Hard, d("2026-10-30")));
    CHECK(tipOf(state, "hard") != std::nullopt);
    CHECK(tipOf(state, "hard", d("2026-11-09")) == std::nullopt);  // ten days later

    addReflection(state, reflection({"choice"}, {}, Feel::Easy));
    CHECK(tipOf(state, "easy") != std::nullopt);
    CHECK(tipOf(state, "hard") == std::nullopt);
}

TEST_CASE("once today's plan is done, the coach offers an extra round of what the learner enjoys", "[coach]")
{
    LearnerState state = learner();
    for (int i = 0; i < 2; ++i)
        addReflection(state, reflection({"conj", "card"}, {"card", "conj"}));
    CHECK(tipOf(state, "enjoy") == std::nullopt);  // the plan comes first

    state.done.push_back({1, kToday});
    const auto enjoy = tipOf(state, "enjoy");
    REQUIRE(enjoy);
    // Flashcards are enjoyed as much, but nothing is due, so the Verb gym it is.
    CHECK(enjoy->trainer == "verbs");
    CHECK(enjoy->title == "An extra round you'll enjoy: the Verb gym");
}

TEST_CASE("ahead of the plan, the coach suggests listening with the spare time", "[coach]")
{
    LearnerState state = learner(d("2026-10-26"));  // five study days ago
    for (int n = 1; n <= 8; ++n)
        state.done.push_back({n, d("2026-10-30")});
    const auto ahead = tipOf(state, "ahead");
    REQUIRE(ahead);
    CHECK(ahead->title == "You are 3 days ahead");
    CHECK(ahead->trainer.empty());
}
