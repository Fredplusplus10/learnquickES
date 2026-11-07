#include "lq/viewing.h"
#include "test_support.h"

#include <catch2/catch_test_macros.hpp>

using namespace lq;
using lq::test::d;

namespace {

// Netflix's "Download all" from Viewing activity: two columns, dates as the account's country writes them.
constexpr std::string_view kNetflixSimple = "Title,Date\n"
                                            "\"Élite: Temporada 1: Bienvenidos\",10/30/26\n"
                                            "\"Élite: Temporada 1: Tarde de copas\",10/31/26\n"
                                            "\"Roma\",11/1/26\n"
                                            "\"Friends: Season 1: The One Where Monica Gets a Roommate\",11/2/26\n";

// The ViewingActivity.csv of Netflix's personal-information download.
constexpr std::string_view kNetflixFull =
    "\xEF\xBB\xBFProfile Name,Start Time,Duration,Attributes,Title,Supplemental Video Type,Device Type,Bookmark,"
    "Latest Bookmark,Country\r\n"
    "Profile 1,2026-11-02 21:10:00,00:41:20,,\"La casa de papel: Parte 1: Episodio 1\",,TV,00:41:20,00:41:20,ES\r\n"
    "Profile 1,2026-11-02 20:30:00,00:10:05,,\"La casa de papel: Parte 1: Episodio 1\",,TV,00:10:05,00:10:05,ES\r\n"
    "Profile 1,2026-11-02 20:28:00,00:01:30,Autoplayed,\"La casa de papel: Parte 1: Episodio 2\",,TV,,,ES\r\n"
    "Profile 1,2026-11-01 19:00:00,00:02:10,,\"La casa de papel: Tráiler\",TRAILER,TV,,,ES\r\n"
    "Kids,2026-11-01 18:00:00,00:22:00,,\"Peppa Pig: Season 1: Muddy Puddles\",,TV,,,ES\r\n";

} // namespace

TEST_CASE("a show is the part of a title before its season, a film is its own show", "[viewing]")
{
    CHECK(showOf("La casa de papel: Parte 1: Episodio 1") == "La casa de papel");
    CHECK(showOf("Roma") == "Roma");
    CHECK(showOf("Star Wars: A New Hope") == "Star Wars: A New Hope");  // one ": " only: a film's subtitle
}

TEST_CASE("shows made in Spanish are known by their Spanish and English titles", "[viewing]")
{
    CHECK(madeInSpanish("La casa de papel"));
    CHECK(madeInSpanish("Money Heist"));
    CHECK(madeInSpanish("ÉLITE"));
    CHECK(madeInSpanish("¿Quién mató a Sara?"));
    CHECK(madeInSpanish(showOf("Luis Miguel: La serie: Temporada 1: Episodio 1")));
    CHECK_FALSE(madeInSpanish("Friends"));
    CHECK_FALSE(madeInSpanish("Stranger Things"));
}

TEST_CASE("Netflix's Title and Date download is read, its dates as the account writes them", "[viewing]")
{
    std::string why;
    const auto file = readViewing(kNetflixSimple, true, &why);
    REQUIRE(file);
    CHECK(file->service == "Netflix");
    CHECK_FALSE(file->hasMinutes);
    REQUIRE(file->items.size() == 4);
    // 10/30/26 can only be month first, so every date in the file is read that way despite dayFirst.
    CHECK(file->items[0].date == d("2026-10-30"));
    CHECK(file->items[2].date == d("2026-11-01"));
    CHECK(file->items[0].show == "Élite");
    CHECK(file->items[2].show == "Roma");

    // Nothing in the file settles 3/4/26: the computer's own order decides.
    const auto european = readViewing("Title,Date\nRoma,3/4/26\n", true);
    const auto american = readViewing("Title,Date\nRoma,3/4/26\n", false);
    REQUIRE((european && american));
    CHECK(european->items[0].date == d("2026-04-03"));
    CHECK(american->items[0].date == d("2026-03-04"));
}

TEST_CASE("Netflix's full export keeps profiles and durations, without trailers or previews", "[viewing]")
{
    const auto file = readViewing(kNetflixFull, false);
    REQUIRE(file);
    CHECK(file->service == "Netflix");
    CHECK(file->hasMinutes);
    REQUIRE(file->items.size() == 3);  // the 1½-minute autoplay and the trailer are left out
    CHECK(file->skipped == 2);
    CHECK(file->items[0].profile == "Profile 1");
    CHECK(file->items[0].minutes == 41);
    CHECK(file->items[1].minutes == 10);
    CHECK(file->items[0].date == d("2026-11-02"));
    CHECK(file->items[2].show == "Peppa Pig");
}

TEST_CASE("any CSV with Title, Date and Minutes columns is read, other files are turned down", "[viewing]")
{
    const auto file = readViewing("title,date,minutes\n\"Narcos, the series: Season 1: Pilot\",2026-11-03,52\n", true);
    REQUIRE(file);
    CHECK(file->service == "TV");
    CHECK(file->items[0].title == "Narcos, the series: Season 1: Pilot");
    CHECK(file->items[0].minutes == 52);

    std::string why;
    CHECK_FALSE(readViewing("name,score\nAna,10\n", true, &why));
    CHECK(why.find("Title and a Date") != std::string::npos);
    CHECK_FALSE(readViewing("", true, &why));
    CHECK_FALSE(readViewing("Title,Date\nRoma,31/2/26\nElite,yesterday\n", true, &why));  // no real date
}

TEST_CASE("imported viewing counts as listening on its day, without XP, and only once", "[viewing]")
{
    LearnerState state = defaultState(d("2026-10-01"));
    const lq::Date today = d("2026-11-03");
    const auto file = readViewing(kNetflixFull, false);
    REQUIRE(file);

    const ViewingImport first = importViewing(state, *file, {"La casa de papel"}, "Profile 1", 0, state.plan.start, today);
    CHECK(first.added == 1);  // one episode, watched in two sittings on one day
    CHECK(first.minutes == 51);
    REQUIRE(state.input.size() == 1);
    CHECK(state.input[0].imported);
    CHECK(state.input[0].kind == "series");
    CHECK(state.input[0].source == "Netflix · La casa de papel: Parte 1: Episodio 1");
    CHECK(state.activity.at(d("2026-11-02")).inputMinutes == 51);
    CHECK(state.activity.at(d("2026-11-02")).xp == 0);  // no XP, so no streak changes after the fact
    CHECK(activeDays(state).empty());
    CHECK(importedShows(state, "Netflix") == std::set<std::string>{"La casa de papel"});

    // The same file again adds nothing.
    const ViewingImport again = importViewing(state, *file, {"La casa de papel"}, "Profile 1", 0, state.plan.start, today);
    CHECK(again.added == 0);
    CHECK(again.already == 1);
    CHECK(state.activity.at(d("2026-11-02")).inputMinutes == 51);

    // Another profile's show is left out for this profile, and counted for all.
    CHECK(importViewing(state, *file, {"Peppa Pig"}, "Profile 1", 0, state.plan.start, today).added == 0);
    CHECK(importViewing(state, *file, {"Peppa Pig"}, "", 0, state.plan.start, today).added == 1);

    // Taking an imported entry back takes its minutes, and no XP it never gave.
    state.activity[d("2026-11-02")].xp = 30;  // a lesson done that day
    removeInput(state, 1);
    CHECK(state.activity.at(d("2026-11-02")).inputMinutes == 0);
    CHECK(state.activity.at(d("2026-11-02")).xp == 30);
}

TEST_CASE("without durations each title counts the minutes the learner gives; only from the start, never ahead", "[viewing]")
{
    LearnerState state = defaultState(d("2026-10-31"));
    const auto file = readViewing(kNetflixSimple, false);
    REQUIRE(file);
    // 10/30 is before the plan started, 11/2 is after today.
    const ViewingImport done = importViewing(state, *file, {"Élite", "Roma", "Friends"}, "", 45, state.plan.start, d("2026-11-01"));
    CHECK(done.added == 2);  // Élite on 10/31 and Roma on 11/1
    CHECK(done.minutes == 90);
    CHECK(state.input.front().date == d("2026-10-31"));

    // Counting what came before the plan as well.
    CHECK(importViewing(state, *file, {"Élite"}, "", 45, Date::min(), d("2026-11-01")).added == 1);
    CHECK(state.input.front().date == d("2026-10-30"));  // the log stays in date order
}
