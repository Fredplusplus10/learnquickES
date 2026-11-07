#include "lq/calendar.h"
#include "test_support.h"

#include <set>
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

using lq::test::readFile;

TEST_CASE("the exported course has the 25 units the schedule expects", "[course]")
{
    const auto course = nlohmann::json::parse(readFile(LQ_CONTENT_DIR "/course.json"));
    REQUIRE(course.at("phases").size() == lq::kPhases);
    REQUIRE(course.at("units").size() == lq::kPhases * lq::kUnitsPerPhase);
    int index = 0;
    for (const auto &unit : course.at("units")) {
        CAPTURE(index);
        CHECK(unit.at("n").get<int>() == index + 1);
        CHECK(unit.at("phase").get<int>() == index / lq::kUnitsPerPhase + 1);
        CHECK_FALSE(unit.at("title").get<std::string>().empty());
        CHECK_FALSE(unit.at("es").get<std::string>().empty());
        ++index;
    }
}

TEST_CASE("the immersion screen has its resources, tips, missions for Windows and Mac, and exchange phrases", "[course]")
{
    const lq::Course &course = lq::test::course();
    const lq::Immersion &immersion = course.immersion;
    CHECK(immersion.resources.size() == 5);
    for (const lq::Resource &r : immersion.resources) {
        CAPTURE(r.name);
        CHECK(r.url.starts_with("https://"));
        CHECK_FALSE(r.note.empty());
    }
    CHECK(immersion.seriesTips.size() == 4);

    // Series for every level the learner goes through, each with somewhere to watch it.
    std::set<std::string> levels;
    for (const lq::Series &s : immersion.series) {
        CAPTURE(s.name);
        CHECK((s.level == "A1" || s.level == "A2" || s.level == "B1"));
        CHECK_FALSE(s.note.empty());
        CHECK_FALSE(s.where.empty());
        levels.insert(s.level);
    }
    CHECK(levels == std::set<std::string>{"A1", "A2", "B1"});
    CHECK(immersion.series.size() == 5);
    CHECK_FALSE(immersion.seriesWhere.empty());
    CHECK(immersion.exchangePhrases.size() == 6);

    // Six devices, so "Goodbye English" (five) never needs all of them; ids the web app uses, so a web backup
    // keeps its ticks.
    REQUIRE(immersion.missions.size() == 6);
    for (const lq::Mission &m : immersion.missions) {
        CAPTURE(m.id);
        CHECK((m.id == "phone" || m.id == "mac" || m.id == "social" || m.id == "youtube" || m.id == "siri"
               || m.id == "keyboard"));
    }
    CHECK(immersion.missions[1].how.find("Windows") != std::string::npos);
    CHECK(immersion.missions[0].how.find("Android") != std::string::npos);
}
