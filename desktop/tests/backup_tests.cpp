// The backup file both apps share. data/web_reference.json holds the sample learner as the web app exports it
// (desktop/tools/web_reference.js).
#include "lq/backup.h"
#include "lq/demo.h"
#include "test_support.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <string>
#include <vector>

using namespace lq;
using lq::test::d;
using nlohmann::json;

namespace {

const std::string &webBackup()
{
    static const std::string text = json::parse(lq::test::readFile(LQ_TEST_DATA_DIR "/web_reference.json")).at("backup").dump();
    return text;
}

Backup read(const std::string &text)
{
    std::string error;
    auto backup = readBackup(text, lq::test::course(), &error);
    INFO(error);
    REQUIRE(backup);
    return *std::move(backup);
}

std::string refused(const std::string &text)
{
    std::string error;
    CHECK_FALSE(readBackup(text, lq::test::course(), &error));
    return error;
}

// Log entries without their ids, which each app makes up for itself.
json withoutIds(json entries)
{
    for (json &entry : entries)
        entry.erase("id");
    return entries;
}

json sorted(json list)
{
    std::sort(list.begin(), list.end());
    return list;
}

std::vector<std::string> keys(const nlohmann::ordered_json &object)
{
    std::vector<std::string> out;
    for (const auto &[key, value] : object.items())
        out.push_back(key);
    return out;
}

// A web backup with only what the web app requires, plus `extra` merged into its state.
std::string minimalBackup(const json &extra = json::object())
{
    json state{{"profile", {{"name", "Ana"}}}, {"plan", {{"start", "2026-11-02"}}}, {"activity", json::object()}};
    state.update(extra);
    return json{{"app", "LearnQuick"}, {"state", state}}.dump();
}

} // namespace

TEST_CASE("a backup from the web app restores its learner, and the desktop app writes it back the same", "[backup]")
{
    const Backup backup = read(webBackup());
    CHECK(backup.profile.name == "Cory");
    CHECK(backup.profile.level == "A2");
    CHECK_FALSE(backup.profile.desktop);
    CHECK(backup.profile.reason.empty());
    CHECK(backup.state.cards.size() == 84);
    CHECK(backup.state.input.size() == 17);
    CHECK(backup.state.exchanges.size() == 1);

    const json web = json::parse(webBackup()).at("state");
    const std::string written = writeBackup(backup, lq::test::course(), "2026-11-03T08:00:00.000Z");
    const json file = json::parse(written);
    CHECK(file.at("app") == "LearnQuick");
    CHECK(file.at("exported") == "2026-11-03T08:00:00.000Z");
    const json &mine = file.at("state");
    for (const char *key : {"done", "tasks", "units", "cards", "activity", "missions", "achievements"}) {
        INFO(key);
        CHECK(mine.at(key) == web.at(key));
    }
    for (const char *key : {"days", "start", "studyDays", "skipHolidays", "daysOff"}) {
        INFO(key);
        CHECK(mine.at("plan").at(key) == web.at("plan").at(key));
    }
    json checkpoints = mine.at("checkpoints");
    for (auto &[phase, score] : checkpoints.items())
        score.erase("attempts");  // only the desktop app counts them
    CHECK(checkpoints == web.at("checkpoints"));
    CHECK(mine.at("streak").at("freezes") == web.at("streak").at("freezes"));
    CHECK(mine.at("streak").at("longest") == web.at("streak").at("longest"));
    CHECK(sorted(mine.at("streak").at("frozen")) == sorted(web.at("streak").at("frozen")));
    CHECK(withoutIds(mine.at("immersion")) == withoutIds(web.at("immersion")));
    CHECK(withoutIds(mine.at("exchanges")) == withoutIds(web.at("exchanges")));
    CHECK(mine.at("profile").at("name") == web.at("profile").at("name"));
    CHECK(mine.at("profile").at("level") == web.at("profile").at("level"));
    // The deck keeps its order, which breaks ties in the review queue.
    const auto ordered = [](const std::string &text) {
        const auto parsed = nlohmann::ordered_json::parse(text);
        return keys(parsed.contains("state") ? parsed.at("state").at("cards") : parsed.at("cards"));
    };
    CHECK(ordered(written) == ordered(webBackup()));
}

TEST_CASE("the desktop app's backup brings back everything it saved", "[backup]")
{
    const Date today = d("2026-11-02");
    Backup backup;
    backup.profile = {"Cory", "A2+", true, "I travel to Mexico for work and want real conversations.", {"work", "travel"},
                      "🌮"};
    backup.state = demoState(today, lq::test::course().unitWordIds());
    backup.state.sessions = 9;
    backup.state.reflectedAt = 6;
    backup.state.reflections = {
        {d("2026-10-30"), "mixed", {"choice", "listen"}, {"listen"}, Feel::Hard, "Me gustó escuchar."},
        {d("2026-11-01"), "coach", {"conj", "speak"}, {}, std::nullopt, ""},
    };
    backup.state.plan.studyDays = {1, 3, 5, 6};
    REQUIRE_FALSE(backup.state.checkpoints.empty());
    backup.state.checkpoints.begin()->second.attempts = 3;
    backup.state.input.push_back({d("2026-11-01"), 42, "series", "Netflix · Élite: Temporada 1: Bienvenidos", true});

    const std::string text = writeBackup(backup, lq::test::course(), "2026-11-02T18:00:00.000Z");
    CHECK(json::parse(text).at("state").at("profile").at("level") == "A2+ / B1\xE2\x88\x92");
    const Backup back = read(text);
    CHECK(back.profile.name == backup.profile.name);
    CHECK(back.profile.level == "A2+");
    CHECK(back.profile.desktop);
    CHECK(back.profile.reason == backup.profile.reason);
    CHECK(back.profile.goals == backup.profile.goals);
    CHECK(back.profile.streakEmoji == backup.profile.streakEmoji);
    CHECK(back.state == backup.state);
}

TEST_CASE("files that are not LearnQuick backups are refused with a reason", "[backup]")
{
    CHECK_FALSE(refused("").empty());
    CHECK_FALSE(refused("not json at all").empty());
    CHECK_FALSE(refused("[1, 2, 3]").empty());
    CHECK_FALSE(refused(R"({"app": "LearnQuick", "state": {"profile": {}, "plan": {}}})").empty());
    const std::string damaged = refused(minimalBackup({{"done", json::array({{{"n", 1}, {"date", "yesterday"}}})}}));
    CHECK(damaged.find("yesterday") != std::string::npos);
    CHECK_FALSE(refused(minimalBackup({{"plan", {{"days", 30}}}})).empty());  // no start date
}

TEST_CASE("a backup with only the parts the web app requires reads as a new learner", "[backup]")
{
    const Backup backup = read(minimalBackup({{"plan", {{"start", "2026-11-02"}, {"days", 45}}}}));
    CHECK(backup.profile.name == "Ana");
    CHECK(backup.profile.level.empty());
    CHECK(backup.state.plan.days == 60);  // as the web app's migrate(): only 30, 60 or 90
    CHECK(backup.state.plan.start == d("2026-11-02"));
    CHECK(backup.state.plan.studyDays == std::set<int>{1, 2, 3, 4, 5});
    CHECK(backup.state.cards.empty());
    CHECK(backup.state.activity.empty());
}

TEST_CASE("logged entries in a backup get the limits the screens give them", "[backup]")
{
    const std::string longText(300, 'a');
    const Backup backup = read(minimalBackup({
        {"plan", {{"start", "2026-11-02"}, {"days", 60}}},
        {"immersion", json::array({{{"date", "2026-11-02"}, {"min", 99999}, {"kind", "series"}, {"source", longText}}})},
        {"exchanges", json::array({{{"date", "2026-11-02"}, {"min", 5000}, {"partner", longText}, {"note", longText}}})},
    }));
    REQUIRE(backup.state.input.size() == 1);
    CHECK(backup.state.input[0].minutes == kMaxLoggedMinutes);
    CHECK(backup.state.input[0].source.size() == kMaxLogText);
    REQUIRE(backup.state.exchanges.size() == 1);
    CHECK(backup.state.exchanges[0].minutes == kMaxLoggedMinutes);
    CHECK(backup.state.exchanges[0].note.size() == kMaxLogText);
}

TEST_CASE("lessons the course does not have, unknown tasks and levels are left out", "[backup]")
{
    const json extra{
        {"profile", {{"name", "Ana"}, {"level", "C1"}}},
        {"units", {{"u01", {{"date", "2026-11-02"}, {"score", 0.9}, {"best", 0.9}, {"attempts", 1}}},
                   {"u99", {{"date", "2026-11-02"}, {"score", 0.5}, {"best", 0.5}, {"attempts", 1}}}}},
        {"cards", {{"old_word", {{"box", 2}, {"due", "2026-11-03"}, {"added", "2026-11-01"}, {"u", "u99"}}},
                   {"hola", {{"box", 9}, {"due", "2026-11-03"}, {"added", "2026-11-01"}, {"last", nullptr}, {"u", "u01"}}}}},
        {"tasks", {{"1", {{"lesson", "2026-11-02"}, {"juggling", "2026-11-02"}}}}},
        {"immersion", json::array({{{"id", "a"}, {"date", "2026-11-02"}, {"min", 0}, {"kind", "podcast"}}})},
    };
    const Backup backup = read(minimalBackup(extra));
    CHECK(backup.profile.level.empty());
    REQUIRE(backup.state.units.size() == 1);
    CHECK(backup.state.units.contains(0));
    REQUIRE(backup.state.cards.size() == 1);
    const Card &hola = backup.state.cards.at("hola");
    CHECK(hola.unit == 0);
    CHECK(hola.box == kMaxBox);
    CHECK(hola.seq == 0);
    CHECK_FALSE(hola.last);
    CHECK(backup.state.tasks.at(1).size() == 1);
    CHECK(backup.state.input.empty());  // nothing logged in zero minutes
}
