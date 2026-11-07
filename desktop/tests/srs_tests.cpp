#include "lq/srs.h"
#include "test_support.h"

#include <catch2/catch_test_macros.hpp>

using namespace lq;
using lq::test::d;

TEST_CASE("a new card starts in box 1 and comes back the next day", "[srs]")
{
    const Card card = newCard(d("2026-11-02"));
    CHECK(card.box == 1);
    CHECK(card.due == d("2026-11-03"));
    CHECK(card.added == d("2026-11-02"));
    CHECK(card.reps == 0);
    CHECK_FALSE(card.last);
    CHECK_FALSE(isDue(card, d("2026-11-02")));
    CHECK(isDue(card, d("2026-11-03")));
}

TEST_CASE("good moves one box, easy two, again back to box 1", "[srs]")
{
    const Date day = d("2026-11-03");
    const Card card = newCard(d("2026-11-02"));

    const Card good = gradeCard(card, Grade::Good, day);
    CHECK(good.box == 2);
    CHECK(good.due == addDays(day, kSrsIntervals[2]));
    CHECK(good.last == day);

    const Card easy = gradeCard(card, Grade::Easy, day);
    CHECK(easy.box == 3);
    CHECK(easy.due == addDays(day, kSrsIntervals[3]));

    Card strong = card;
    strong.box = kMaxBox;
    CHECK(gradeCard(strong, Grade::Easy, day).box == kMaxBox);  // capped
    CHECK(gradeCard(strong, Grade::Easy, day).due == addDays(day, 60));

    const Card again = gradeCard(strong, Grade::Again, day);
    CHECK(again.box == 1);
    CHECK(again.lapses == 1);
    CHECK(again.due == addDays(day, 1));
    CHECK_FALSE(remembered(again));
    CHECK(remembered(gradeCard(easy, Grade::Good, day)));  // box 4
}

TEST_CASE("the review queue takes due cards only, most overdue first, up to the limit", "[srs]")
{
    Deck deck;
    const auto add = [&deck](const char *id, int box, const char *due) {
        Card card = newCard(d("2026-10-01"));
        card.box = box;
        card.due = d(due);
        card.seq = static_cast<int>(deck.size());
        deck.emplace(id, card);
    };
    add("later", 1, "2026-11-05");
    add("today", 1, "2026-11-02");
    add("old", 3, "2026-10-20");
    add("old-weak", 1, "2026-10-20");

    CHECK(reviewQueue(deck, d("2026-11-02")) == std::vector<std::string>{"old-weak", "old", "today"});
    CHECK(reviewQueue(deck, d("2026-11-02"), 1) == std::vector<std::string>{"old-weak"});
    CHECK(reviewQueue({}, d("2026-11-02")).empty());
}
