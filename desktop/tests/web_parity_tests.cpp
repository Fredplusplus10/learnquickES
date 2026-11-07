// Golden-master tests: the web app's engine produced data/web_reference.json
// (desktop/tools/web_reference.js); the C++ engine must give the same answers.
#include "lq/demo.h"
#include "lq/exercises.h"
#include "lq/srs.h"
#include "lq/text.h"
#include "lq/verbs.h"
#include "test_support.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

using namespace lq;
using lq::test::d;
using nlohmann::json;

namespace {

const json &reference()
{
    static const json data = json::parse(lq::test::readFile(LQ_TEST_DATA_DIR "/web_reference.json"));
    return data;
}

std::vector<std::string> isoList(const std::set<Date> &dates)
{
    std::vector<std::string> out;
    for (const Date date : dates)
        out.push_back(toIso(date));
    return out;
}

std::string typeName(DayType type)
{
    switch (type) {
    case DayType::Lesson: return "lesson";
    case DayType::Practice: return "practice";
    case DayType::Checkpoint: return "checkpoint";
    }
    return "";
}

std::string stateName(DayState state)
{
    switch (state) {
    case DayState::Done: return "done";
    case DayState::Frozen: return "frozen";
    case DayState::Future: return "future";
    case DayState::Today: return "today";
    case DayState::Missed: return "missed";
    case DayState::Off: return "off";
    }
    return "";
}

} // namespace

TEST_CASE("holidays match the web app for 2020-2040", "[parity]")
{
    for (const auto &[year, dates] : reference().at("holidays").items()) {
        CAPTURE(year);
        CHECK(isoList(usFederalHolidays(std::stoi(year))) == dates.get<std::vector<std::string>>());
    }
}

TEST_CASE("schedules and daily tasks match the web app", "[parity]")
{
    for (const auto &[days, entries] : reference().at("schedules").items()) {
        const int total = std::stoi(days);
        const auto schedule = buildSchedule(total);
        REQUIRE(schedule.size() == entries.size());
        for (std::size_t i = 0; i < schedule.size(); ++i) {
            const auto &want = entries[i];
            const auto &got = schedule[i];
            CAPTURE(total, got.n);
            CHECK(got.n == want.at("n").get<int>());
            CHECK(got.phase == want.at("phase").get<int>());
            CHECK(typeName(got.type) == want.at("type").get<std::string>());
            CHECK(got.unit == want.at("unit").get<int>());
            CHECK(got.practiceIndex == want.at("practice").get<int>());
            std::vector<std::string> tasks;
            for (const Task task : dayTasks(got, total))
                tasks.emplace_back(taskId(task));
            CHECK(tasks == want.at("tasks").get<std::vector<std::string>>());
        }
    }
}

TEST_CASE("the demo learner matches the web app's on several dates", "[parity]")
{
    for (const auto &[iso, want] : reference().at("demo").items()) {
        CAPTURE(iso);
        const Date today = d(iso.c_str());
        const LearnerState state = demoState(today, lq::test::course().unitWordIds());

        CHECK(toIso(state.plan.start) == want.at("start").get<std::string>());

        std::vector<std::string> done;
        for (const DoneDay &day : state.done)
            done.push_back(toIso(day.date));
        CHECK(done == want.at("done").get<std::vector<std::string>>());

        std::vector<int> units;
        for (const auto &[unit, score] : state.units)
            units.push_back(unit);
        CHECK(units == want.at("units").get<std::vector<int>>());

        std::vector<int> checkpoints;
        for (const auto &[phase, score] : state.checkpoints)
            checkpoints.push_back(phase);
        CHECK(checkpoints == want.at("checkpoints").get<std::vector<int>>());

        std::map<std::string, int> xp;
        for (const auto &[date, activity] : state.activity)
            xp[toIso(date)] = activity.xp;
        CHECK(xp == want.at("xp").get<std::map<std::string, int>>());

        for (const auto &[day, wantDay] : want.at("activity").items()) {
            CAPTURE(day);
            const auto found = state.activity.find(d(day.c_str()));
            REQUIRE(found != state.activity.end());
            const Activity &a = found->second;
            CHECK(a.seconds == wantDay.at("sec").get<int>());
            CHECK(a.inputMinutes == wantDay.at("input").get<int>());
            CHECK(a.islandMinutes == wantDay.at("island").get<int>());
            CHECK(a.speak == wantDay.at("speak").get<int>());
            std::map<std::string, std::vector<int>> answers;
            for (const auto &[skill, tally] : a.answers)
                answers[skill] = {tally.correct, tally.total};
            CHECK(answers == wantDay.at("ex").get<std::map<std::string, std::vector<int>>>());
        }

        // The deck in the order cards joined it, then the review queue it gives today.
        std::vector<std::pair<std::string, Card>> cards(state.cards.begin(), state.cards.end());
        std::sort(cards.begin(), cards.end(), [](const auto &a, const auto &b) { return a.second.seq < b.second.seq; });
        const json &wantCards = want.at("cards");
        REQUIRE(cards.size() == wantCards.size());
        for (std::size_t i = 0; i < cards.size(); ++i) {
            CAPTURE(i);
            const auto &[id, card] = cards[i];
            CHECK(id == wantCards[i].at("id").get<std::string>());
            CHECK(card.unit == wantCards[i].at("unit").get<int>());
            CHECK(card.box == wantCards[i].at("box").get<int>());
            CHECK(toIso(card.due) == wantCards[i].at("due").get<std::string>());
            CHECK(toIso(card.added) == wantCards[i].at("added").get<std::string>());
            CHECK(card.reps == wantCards[i].at("reps").get<int>());
        }
        CHECK(reviewQueue(state.cards, today) == want.at("queue").get<std::vector<std::string>>());

        CHECK(state.freezes == want.at("freezes").get<int>());
        CHECK(isoList(state.frozen) == want.at("frozen").get<std::vector<std::string>>());

        const StreakInfo streak = streakInfo(state, today);
        CHECK(streak.count == want.at("streak").get<int>());
        CHECK(streak.longest == want.at("longest").get<int>());
        std::vector<std::string> week;
        for (const WeekDay &day : streak.week)
            week.push_back(stateName(day.state));
        CHECK(week == want.at("week").get<std::vector<std::string>>());

        const Pace p = pace(state, today);
        const json &wantPace = want.at("pace");
        CHECK(p.expected == wantPace.at("expected").get<int>());
        CHECK(p.diff == wantPace.at("diff").get<int>());
        CHECK(p.remaining == wantPace.at("remaining").get<int>());
        REQUIRE(p.target);
        CHECK(toIso(*p.target) == wantPace.at("target").get<std::string>());
    }
}

TEST_CASE("answers are checked like the web app: case, punctuation and accents", "[parity]")
{
    for (const json &c : reference().at("answers")) {
        const auto input = c.at("input").get<std::string>();
        CAPTURE(input);
        const AnswerCheck got = checkAnswer(input, c.at("answers").get<std::vector<std::string>>());
        const json &want = c.at("result");
        CHECK(got.ok == want.at("ok").get<bool>());
        CHECK(got.almost == want.value("almost", false));
        CHECK(got.empty == want.value("empty", false));
        CHECK(got.expected == want.at("expected").get<std::string>());
    }
    for (const auto &[input, want] : reference().at("normalize").items()) {
        CAPTURE(input);
        CHECK(normalize(input) == want.get<std::string>());
    }
}

TEST_CASE("every verb in every tense conjugates like the web app", "[parity]")
{
    const json &all = reference().at("verbs");
    REQUIRE(all.size() == verbs().size());
    for (const auto &[infinitive, forms] : all.items()) {
        CAPTURE(infinitive);
        REQUIRE(findVerb(infinitive) != nullptr);
        CHECK(participle(infinitive) == forms.at("participle").get<std::string>());
        for (const auto &[id, want] : forms.items()) {
            if (id == "participle")
                continue;
            CAPTURE(id);
            const auto tense = tenseFromId(id);
            REQUIRE(tense);
            const Paradigm got = conjugate(infinitive, *tense);
            CHECK(std::vector<std::string>(got.begin(), got.end()) == want.get<std::vector<std::string>>());
        }
    }
    std::vector<std::string> core;
    for (const std::string_view verb : coreVerbs())
        core.emplace_back(verb);
    CHECK(core == reference().at("verbCore").get<std::vector<std::string>>());
}

TEST_CASE("spaced repetition schedules and orders cards like the web app", "[parity]")
{
    const json &srs = reference().at("srs");

    Card card = newCard(d("2026-11-02"));
    for (const json &step : srs.at("steps")) {
        const Date day = card.due;
        CAPTURE(toIso(day));
        CHECK(toIso(day) == step.at("day").get<std::string>());
        card = gradeCard(card, static_cast<Grade>(step.at("grade").get<int>()), day);
        CHECK(card.box == step.at("box").get<int>());
        CHECK(toIso(card.due) == step.at("due").get<std::string>());
        CHECK(card.reps == step.at("reps").get<int>());
        CHECK(card.lapses == step.at("lapses").get<int>());
    }

    Deck deck;
    for (const json &c : srs.at("deck")) {
        Card added = newCard(d("2026-10-01"));
        added.box = c.at("box").get<int>();
        added.due = d(c.at("due").get<std::string>().c_str());
        added.seq = static_cast<int>(deck.size());
        deck.emplace(c.at("id").get<std::string>(), added);
    }
    CHECK(reviewQueue(deck, d("2026-11-02"), 20) == srs.at("queue").get<std::vector<std::string>>());
    CHECK(reviewQueue(deck, d("2026-11-02"), 3) == srs.at("queue3").get<std::vector<std::string>>());
}

TEST_CASE("the B1 mock's writing checks give the web app's verdicts and labels", "[parity]")
{
    for (const json &c : reference().at("writing")) {
        const auto text = c.at("text").get<std::string>();
        CAPTURE(text);
        const std::vector<WritingCheck> got = writingChecks(text);
        const json &want = c.at("checks");
        REQUIRE(got.size() == want.size());
        for (std::size_t i = 0; i < got.size(); ++i) {
            CAPTURE(i);
            CHECK(got[i].ok == want[i].at("ok").get<bool>());
            CHECK(got[i].label == want[i].at("label").get<std::string>());
        }
    }
}
