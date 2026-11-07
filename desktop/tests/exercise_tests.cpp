#include "lq/exercises.h"
#include "lq/text.h"
#include "test_support.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <set>

using namespace lq;
using lq::test::course;
using lq::test::d;

namespace {

// Picks the tiles that spell the sentence, the way a learner who knows it would.
std::vector<int> solveTiles(const Item &item)
{
    std::string spaced = item.es;
    for (const std::string_view mark : {"¿", "?", "¡", "!", ".", ",", ";", ":"}) {
        for (std::size_t at = spaced.find(mark); at != std::string::npos; at = spaced.find(mark, at))
            spaced.replace(at, mark.size(), " ");
    }
    std::vector<int> picked;
    std::size_t start = 0;
    bool first = true;
    while (start < spaced.size()) {
        const std::size_t end = std::min(spaced.find(' ', start), spaced.size());
        std::string word = spaced.substr(start, end - start);
        start = end + 1;
        if (word.empty())
            continue;
        if (first)
            word = toLower(word);
        first = false;
        for (int i = 0; i < static_cast<int>(item.tiles.size()); ++i) {
            if (item.tiles[static_cast<std::size_t>(i)] == word && std::find(picked.begin(), picked.end(), i) == picked.end()) {
                picked.push_back(i);
                break;
            }
        }
    }
    return picked;
}

Item choice(const char *question)
{
    Item item;
    item.type = ItemType::Choice;
    item.question = question;
    item.options = {"right", "wrong"};
    item.answer = 0;
    return item;
}

} // namespace

TEST_CASE("every lesson builds a practice session a learner can get fully right", "[exercises]")
{
    SeededRandom rnd(42u);
    for (int unit = 0; unit < static_cast<int>(course().units.size()); ++unit) {
        CAPTURE(unit);
        const std::vector<Item> items = lessonItems(course(), unit, rnd);
        REQUIRE(items.size() >= 8);
        CHECK(items.front().type == ItemType::Match);
        CHECK(items.front().pairs.size() == 5);
        for (const Item &item : items) {
            CHECK(item.unit == unit);
            CHECK_FALSE(item.skill.empty());
            Response right;
            switch (item.type) {
            case ItemType::Choice:
                REQUIRE(item.answer >= 0);
                REQUIRE(item.answer < static_cast<int>(item.options.size()));
                right.choice = item.answer;
                break;
            case ItemType::Fill:
            case ItemType::Type:
                REQUIRE_FALSE(item.answers.empty());
                right.text = item.answers.front();
                break;
            case ItemType::Order:
            case ItemType::Listen:
                right.picked = solveTiles(item);
                break;
            default:
                continue;  // matched, chosen or rated on screen, not checked
            }
            CAPTURE(item.question, item.es);
            CHECK(canCheck(item, right));
            const AnswerCheck result = checkItem(item, right);
            CHECK(result.ok);
            CHECK_FALSE(result.almost);
        }
    }
}

TEST_CASE("checkpoints ask 15 questions from their own phase", "[exercises]")
{
    SeededRandom rnd(7u);
    for (int phase = 1; phase < kPhases; ++phase) {
        CAPTURE(phase);
        const std::vector<Item> items = checkpointItems(course(), phase, rnd);
        CHECK(items.size() == 15);
        for (const Item &item : items) {
            if (item.unit >= 0)
                CHECK(course().units.at(static_cast<std::size_t>(item.unit)).phase == phase);
        }
    }
}

TEST_CASE("conjugation items ask unlocked tenses and expect the conjugator's form", "[exercises]")
{
    LearnerState state = defaultState(d("2026-11-02"));
    CHECK(unlockedTenses(state) == std::vector<Tense>{Tense::Present});
    state.units[tenseInfo(Tense::Preterite).unit] = Score{};
    const std::vector<Tense> tenses = unlockedTenses(state);
    CHECK(std::find(tenses.begin(), tenses.end(), Tense::Preterite) != tenses.end());

    SeededRandom rnd(3u);
    for (const Item &item : conjugationItems(coreVerbs(), tenses, 20, false, rnd)) {
        CAPTURE(item.verb, item.person);
        CHECK(item.person != 4);  // no vosotros: the course is Latin-American
        CHECK(std::find(tenses.begin(), tenses.end(), item.tense) != tenses.end());
        CHECK(item.answers == std::vector<std::string>{conjugate(item.verb, item.tense)[static_cast<std::size_t>(item.person)]});
        CHECK_FALSE(item.pronoun.empty());
    }
}

TEST_CASE("every trainer has material from the course", "[exercises]")
{
    SeededRandom rnd(11u);
    CHECK(pastItems(course(), 10, rnd).size() == 10);
    CHECK(cognateItems(course(), 10, rnd).size() == 10);
    CHECK(falseFriendItems(course(), 8, rnd).size() == 8);
    // Each trainer's answers count towards its own skill, so Practice can show them apart.
    for (const Item &item : cognateItems(course(), 10, rnd))
        CHECK(item.skill == "cognates");
    for (const Item &item : falseFriendItems(course(), 8, rnd))
        CHECK(item.skill == "falsefriends");
    CHECK(genderItems(course(), 12, rnd).size() == 12);
    CHECK(describeItems(course(), 5, rnd).size() == 5);
    CHECK(mixedItems(course(), {0, 1, 2}, 10, rnd).size() == 10);
    CHECK(mockGrammarItems(course(), rnd).size() == 10);
    CHECK(mockQuestions(course().mock.reading, "reading", rnd).size() == course().mock.reading.questions.size());

    // Speaking: the day's pronunciation group is the same all day, and differs from day to day over a week.
    CHECK(pronunciationGroupFor(course(), d("2026-11-02")) == pronunciationGroupFor(course(), d("2026-11-02")));
    std::set<int> groups;
    for (int i = 0; i < 7; ++i)
        groups.insert(pronunciationGroupFor(course(), addDays(d("2026-11-02"), i)));
    CHECK(groups.size() > 1);
    CHECK(speakingItems(course(), {}, d("2026-11-02"), rnd).size() >= 4);  // groups have 4 to 6 phrases

    // Review cards carry the word they ask about.
    const std::vector<Item> cards = cardItems(course(), {course().units[0].vocab[0].id});
    REQUIRE(cards.size() == 1);
    CHECK(cards[0].es == course().units[0].vocab[0].es);
}

TEST_CASE("first misses come back in a second-try round at the end; the score counts first tries", "[exercises]")
{
    SeededRandom rnd(1u);
    Session session({choice("a"), choice("b"), choice("c")});
    REQUIRE(session.size() == 3);

    CHECK(session.answer(false, rnd));  // "a" missed: kept for the second try
    CHECK(session.size() == 3);         // the first round keeps its length
    session.next();
    CHECK_FALSE(session.answer(true, rnd));  // "b" right
    session.next();
    CHECK(session.answer(false, rnd));  // "c" missed
    CHECK_FALSE(session.secondTry());
    CHECK(session.roundPosition() == 2);
    CHECK(session.roundSize() == 3);
    session.next();

    // The second-try round: the misses in the order they were missed, counted as a round of their own.
    REQUIRE_FALSE(session.finished());
    CHECK(session.secondTry());
    CHECK(session.roundPosition() == 0);
    CHECK(session.roundSize() == 2);
    CHECK(session.current().retry);
    CHECK(session.current().question == "a");
    CHECK_FALSE(session.answer(true, rnd));
    session.next();
    CHECK(session.current().question == "c");
    CHECK(session.roundPosition() == 1);
    CHECK_FALSE(session.answer(false, rnd));  // a second miss does not come back again
    session.next();
    CHECK(session.finished());

    CHECK(session.answered() == 3);
    CHECK(session.firstTry() == 1);
    CHECK(session.score() == 1.0 / 3.0);
    CHECK(session.secondTries() == 2);
    CHECK(session.fixed() == 1);
    REQUIRE(session.missed().size() == 2);
    CHECK(session.missed()[0].question == "a");
}

TEST_CASE("a session with no misses has no second-try round", "[exercises]")
{
    SeededRandom rnd(1u);
    Session session({choice("a")});
    CHECK_FALSE(session.answer(true, rnd));
    session.next();
    CHECK(session.finished());
    CHECK_FALSE(session.secondTry());
    CHECK(session.secondTries() == 0);
}

TEST_CASE("checkpoints give no second tries; empty sessions score full marks", "[exercises]")
{
    SeededRandom rnd(1u);
    Session checkpoint({choice("a")}, false);
    CHECK_FALSE(checkpoint.answer(false, rnd));
    checkpoint.next();
    CHECK(checkpoint.finished());
    CHECK(checkpoint.score() == 0.0);

    const Session empty;
    CHECK(empty.finished());
    CHECK(empty.score() == 1.0);
}

TEST_CASE("the B1 mock passes with 60% in both the written and the oral group", "[exercises]")
{
    MockScores all;
    all.reading = {5, 5};
    all.grammar = {10, 10};
    all.listening = {5, 5};
    all.writingPoints = 7;
    all.speaking = {3, 3, 3, 3};
    const MockResult full = mockResult(all);
    CHECK(full.overall == 1.0);
    CHECK(full.pass);

    MockScores weakOral = all;
    weakOral.listening = {1, 5};
    weakOral.speaking = {1, 1, 1, 1};
    const MockResult oral = mockResult(weakOral);
    CHECK(oral.written == 1.0);
    CHECK(oral.oral < 0.6);
    CHECK_FALSE(oral.pass);

    CHECK_FALSE(mockResult(MockScores{}).pass);  // nothing answered
}
