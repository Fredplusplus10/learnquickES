#include "lq/progress.h"
#include "test_support.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

using namespace lq;
using lq::test::d;

namespace {

int countEvents(const Events &events, EventType type)
{
    return static_cast<int>(std::count_if(events.begin(), events.end(), [type](const Event &e) { return e.type == type; }));
}

} // namespace

TEST_CASE("a learner can complete the whole 30-day plan; streak, pace and freezes behave", "[progress]")
{
    LearnerState state = defaultState(d("2026-11-02"));
    state.plan.days = 30;
    const std::vector<std::vector<std::string>> words = lq::test::course().unitWordIds();
    Events events;
    const auto record = [&events](const Events &more) { events.insert(events.end(), more.begin(), more.end()); };

    Date day = d("2026-11-02");
    int studied = 0;
    while (!planStatus(state).finished) {
        if (isStudyDay(day, state.plan)) {
            const PlanStatus status = planStatus(state);
            record(addXp(state, kXpCorrect, day));  // one correct answer
            if (status.entry.type == DayType::Checkpoint)
                record(saveCheckpoint(state, status.entry.phase, 0.8, day));
            for (const Task task : status.tasks) {
                if (task == Task::Lesson)
                    record(learnUnit(state, status.entry.unit, words.at(static_cast<std::size_t>(status.entry.unit)), 0.9, day));
                else if (task != Task::Checkpoint && task != Task::Final)
                    record(markTask(state, task, day));
            }
            ++studied;
        }
        day = addDays(day, 1);
        REQUIRE(studied <= 30);  // the plan finishes in 30 study days
    }

    CHECK(state.done.size() == 30);
    CHECK(state.units.size() == 25);
    CHECK(state.cards.size() == 300);  // every word of the course, once
    CHECK(state.checkpoints.size() == 5);
    CHECK(countEvents(events, EventType::DayComplete) == 30);
    CHECK(countEvents(events, EventType::PhaseComplete) == 5);
    CHECK(countEvents(events, EventType::Streak) == 30);  // one per day

    const Date last = state.done.back().date;
    CHECK(streakInfo(state, last).count == 30);  // weekends and Thanksgiving never broke it
    CHECK(pace(state, addDays(last, 1)).remaining == 0);
    CHECK(pace(state, addDays(last, 1)).target == last);

    // A missed study day with a freeze in hand is covered when the app next opens.
    state.freezes = 1;
    const Date missed = nthStudyDay(addDays(last, 1), 1, state.plan).value();
    const Date back = nthStudyDay(addDays(last, 1), 2, state.plan).value();
    const Events opened = applyFreezes(state, back);
    REQUIRE(opened.size() == 1);
    CHECK(opened[0].type == EventType::FreezeUsed);
    CHECK(opened[0].dates == std::vector<Date>{missed});
    CHECK(state.freezes == 0);
    CHECK(streakInfo(state, back).count == 30);
}

TEST_CASE("pace compares completed days with elapsed study days", "[progress]")
{
    LearnerState state = defaultState(d("2026-11-02"));
    const Pace p = pace(state, d("2026-11-09"));  // 5 study days elapsed (Mon–Fri), none done
    CHECK(p.expected == 5);
    CHECK(p.diff == -5);
    CHECK(p.remaining == 60);
    CHECK(p.target == nthStudyDay(d("2026-11-09"), 60, state.plan));

    const Pace fresh = pace(state, d("2026-11-02"));  // first day: nothing expected yet
    CHECK(fresh.expected == 0);
    CHECK(fresh.diff == 0);
}

TEST_CASE("tasks only count for the current plan day", "[progress]")
{
    LearnerState state = defaultState(d("2026-11-02"));
    CHECK(markTask(state, Task::Mixed, d("2026-11-02")).empty());  // not on a lesson day
    CHECK_FALSE(taskDone(state, 1, Task::Mixed));
    CHECK(markTask(state, Task::Review, d("2026-11-02")).empty());
    CHECK(taskDone(state, 1, Task::Review));
    CHECK(planStatus(state).doneCount == 0);
}

TEST_CASE("the week strip shows done, frozen, missed and future days", "[progress]")
{
    LearnerState state = defaultState(d("2026-11-02"));
    addXp(state, 20, d("2026-11-02"));
    state.frozen.insert(d("2026-11-03"));
    const StreakInfo info = streakInfo(state, d("2026-11-05"));
    CHECK(info.week[0].state == DayState::Done);
    CHECK(info.week[1].state == DayState::Frozen);
    CHECK(info.week[2].state == DayState::Missed);
    CHECK(info.week[3].state == DayState::Today);
    CHECK(info.week[4].state == DayState::Future);
    CHECK(info.week[0].date == d("2026-11-02"));
    CHECK_FALSE(info.week[5].scheduled);  // Saturday
}

TEST_CASE("the recommended plan follows the self-assessed level", "[progress]")
{
    CHECK(recommendedPlanDays("A1") == 90);
    CHECK(recommendedPlanDays("A2") == 60);
    CHECK(recommendedPlanDays("A2+") == 30);
    CHECK(recommendedPlanDays("") == 60);
}

TEST_CASE("the series level starts at the welcome level and rises with the phase 2 and 4 checkpoints", "[progress]")
{
    LearnerState state = defaultState(d("2026-11-02"));
    CHECK(seriesLevel("A1", state) == "A1");
    CHECK(seriesLevel("", state) == "A1");
    CHECK(seriesLevel("A2", state) == "A2");
    CHECK(seriesLevel("A2+", state) == "A2");

    state.checkpoints[2] = {d("2026-11-20"), 0.6, 0.65, 1};
    CHECK(seriesLevel("A1", state) == "A1");  // 65% is not a pass
    state.checkpoints[2].best = 0.7;
    CHECK(seriesLevel("A1", state) == "A2");
    state.checkpoints[4] = {d("2026-12-20"), 0.9, 0.9, 1};
    CHECK(seriesLevel("A1", state) == "B1");
    state.checkpoints.erase(2);
    CHECK(seriesLevel("A1", state) == "B1");  // phase 4 shows B1 on its own
}

TEST_CASE("answers earn XP by how right they were and count for their skill", "[progress]")
{
    LearnerState state = defaultState(d("2026-11-02"));
    const Date day = d("2026-11-02");
    recordAnswer(state, "verbs", true, false, false, day);
    CHECK(state.activity[day].xp == kXpCorrect);
    recordAnswer(state, "verbs", true, true, false, day);  // right but for the accents
    CHECK(state.activity[day].xp == kXpCorrect + kXpAlmost);
    recordAnswer(state, "verbs", true, false, true, day);  // right on the second try
    CHECK(state.activity[day].xp == kXpCorrect + kXpAlmost + kXpRetry);
    recordAnswer(state, "verbs", false, false, false, day);
    CHECK(state.activity[day].xp == kXpCorrect + kXpAlmost + kXpRetry);
    CHECK(state.activity[day].answers["verbs"].correct == 3);
    CHECK(state.activity[day].answers["verbs"].total == 4);

    recordPhrase(state, day);
    CHECK(state.activity[day].speak == 1);
    CHECK(state.activity[day].xp == kXpCorrect + kXpAlmost + kXpRetry + kXpPhrase);
}

TEST_CASE("a lesson adds its words to the deck once, due the next day", "[progress]")
{
    LearnerState state = defaultState(d("2026-11-02"));
    const std::vector<std::string> words{"hola", "adios", "gracias"};
    learnUnit(state, 0, words, 0.8, d("2026-11-02"));
    REQUIRE(state.cards.size() == 3);
    CHECK(state.cards.at("hola").due == d("2026-11-03"));
    CHECK(state.cards.at("gracias").seq == 2);
    CHECK(state.cards.at("adios").unit == 0);
    CHECK(state.units.at(0).best == 0.8);

    // Repeating the lesson keeps the cards' progress and the best score.
    state.cards.at("hola").box = 4;
    learnUnit(state, 0, words, 0.6, d("2026-11-03"));
    CHECK(state.cards.size() == 3);
    CHECK(state.cards.at("hola").box == 4);
    CHECK(state.units.at(0).best == 0.8);
    CHECK(state.units.at(0).attempts == 2);
}

TEST_CASE("grading review cards moves them through the boxes and earns XP", "[progress]")
{
    LearnerState state = defaultState(d("2026-11-02"));
    const Date day = d("2026-11-02");
    gradeReview(state, "hola", 0, Grade::Good, day);
    REQUIRE(state.cards.contains("hola"));
    CHECK(state.cards.at("hola").box == 2);
    CHECK(state.activity[day].xp == kXpCard);
    gradeReview(state, "hola", 0, Grade::Again, day);
    CHECK(state.cards.at("hola").box == 1);
    CHECK(state.cards.at("hola").lapses == 1);
    CHECK(state.activity[day].xp == kXpCard + 1);
    CHECK(state.activity[day].answers["review"].total == 2);
    CHECK(state.activity[day].answers["review"].correct == 1);
}

TEST_CASE("review ticks itself when nothing is due and unticks when cards come due", "[progress]")
{
    LearnerState state = defaultState(d("2026-11-02"));

    // Day 1, empty deck: nothing to review, so the task is done.
    ReviewSync sync = syncReview(state, d("2026-11-02"));
    CHECK(sync.changed);
    CHECK(taskDone(state, 1, Task::Review));
    CHECK_FALSE(syncReview(state, d("2026-11-02")).changed);

    // The lesson adds cards due tomorrow; the plan day is still day 1 tomorrow (lesson not finished yet
    // in this test), so tomorrow the earlier tick no longer holds.
    Card card = newCard(d("2026-11-02"));
    state.cards.emplace("hola", card);
    sync = syncReview(state, d("2026-11-03"));
    CHECK(sync.changed);
    CHECK_FALSE(taskDone(state, 1, Task::Review));

    // A tick made today stays even with cards due: the learner reviewed them today.
    markTask(state, Task::Review, d("2026-11-03"));
    CHECK_FALSE(syncReview(state, d("2026-11-03")).changed);
    CHECK(taskDone(state, 1, Task::Review));
}

TEST_CASE("listening, the language island and conversations count as Spanish time and earn XP", "[progress]")
{
    LearnerState state = defaultState(d("2026-11-02"));
    const Date today = d("2026-11-02");
    const Events events = logInput(state, 25, "podcast", "  No Hay Tos  ", today);
    CHECK(countEvents(events, EventType::Streak) == 1);  // 25 XP is a day's streak
    logInput(state, 30, "island", "", today);
    logInput(state, 9999, "telepathy", std::string(300, 'x'), today);  // clamped, unknown kind, cut
    logExchange(state, 45, "Mariana (Tandem)", "Hablamos de comida.", today);

    const Activity &a = state.activity.at(today);
    CHECK(a.inputMinutes == 25 + kMaxLoggedMinutes);
    CHECK(a.islandMinutes == 30);
    CHECK(a.talkMinutes == 45);
    CHECK(a.xp == 25 + 30 + kMaxLoggedMinutes + 45);
    REQUIRE(state.input.size() == 3);
    CHECK(state.input[0].source == "No Hay Tos");
    CHECK(state.input[2].kind == "other");
    CHECK(state.input[2].source.size() == kMaxLogText);
    CHECK(spanishMinutes(state, today) == 25 + 30 + kMaxLoggedMinutes + 45);

    // Taking an entry back takes back its minutes and XP.
    CHECK(removeInput(state, 2));
    CHECK_FALSE(removeInput(state, 5));
    CHECK(state.activity.at(today).inputMinutes == 25);
    CHECK(state.activity.at(today).xp == 25 + 30 + 45);
    CHECK(removeInput(state, 1));
    CHECK(state.activity.at(today).islandMinutes == 0);

    setMission(state, "phone", true, today);
    setMission(state, "phone", true, d("2026-11-05"));  // already done: keeps its date
    setMission(state, "social", true, today);
    setMission(state, "social", false, today);
    REQUIRE(state.missions.size() == 1);
    CHECK(state.missions.at("phone") == today);
}

TEST_CASE("totals add up every day, every skill and the deck", "[progress]")
{
    LearnerState state = defaultState(d("2026-11-02"));
    recordAnswer(state, "verbs", true, false, false, d("2026-11-02"));
    recordAnswer(state, "verbs", false, false, false, d("2026-11-03"));
    state.activity[d("2026-11-03")].seconds = 600;
    logInput(state, 20, "video", "", d("2026-11-03"));
    learnUnit(state, 0, {"a", "b", "c"}, 1.0, d("2026-11-02"));
    state.cards.at("a").box = kRememberedBox;

    const Totals t = totals(state, d("2026-11-03"));
    CHECK(t.answered("verbs") == 2);
    CHECK(t.accuracy("verbs") == 0.5);
    CHECK_FALSE(t.accuracy("past").has_value());
    CHECK(t.appMinutes == 10.0);
    CHECK(t.spanishMinutes() == 30.0);
    CHECK(t.wordsSeen == 3);
    CHECK(t.wordsRemembered == 1);
    CHECK(t.wordsDue == 3);  // added on the 2nd, due the next day
}

TEST_CASE("listening hours climb Dreaming Spanish's milestones", "[progress]")
{
    CHECK(listeningLevel(0).level == 1);
    CHECK(listeningLevel(0).nextHours == 50);
    CHECK(listeningLevel(25 * 60).progress == 0.5);
    CHECK(listeningLevel(50 * 60).level == 2);
    CHECK(listeningLevel(50 * 60).nextHours == 150);
    CHECK(listeningLevel(1499 * 60).level == 6);
    const ListeningLevel last = listeningLevel(1600 * 60);
    CHECK(last.level == 7);
    CHECK(last.nextHours == 0);
    CHECK(last.progress == 1.0);
}
