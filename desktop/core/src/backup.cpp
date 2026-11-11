#include "lq/backup.h"

#include "lq/text.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace lq {

namespace {

// Ordered, as the web app writes objects: the card order is the deck's order.
using json = nlohmann::ordered_json;

struct BadFile {
    std::string why;
};

Date dateIn(const json &value)
{
    if (!value.is_string())
        throw BadFile{"a date is missing"};
    const auto date = parseIso(value.get<std::string>());
    if (!date)
        throw BadFile{"\"" + value.get<std::string>() + "\" is not a date"};
    return *date;
}

std::optional<Date> optionalDate(const json &object, const char *key)
{
    const auto it = object.find(key);
    if (it == object.end() || it->is_null())
        return std::nullopt;
    return dateIn(*it);
}

// Every count in a backup is zero or more and fits an int. A number that does not is a damaged file: converting
// it would be undefined, and cutting it to fit would restore progress nobody made.
int countOf(const json &value)
{
    const double n = value.get<double>();
    if (!std::isfinite(n) || n < 0 || n > static_cast<double>(std::numeric_limits<int>::max()))
        throw BadFile{"a number is out of range"};
    return static_cast<int>(n);
}

int intIn(const json &object, const char *key, int fallback = 0)
{
    const auto it = object.find(key);
    return it != object.end() && it->is_number() ? countOf(*it) : fallback;
}

double numberIn(const json &object, const char *key)
{
    const auto it = object.find(key);
    return it != object.end() && it->is_number() ? it->get<double>() : 0.0;
}

std::string textIn(const json &object, const char *key)
{
    const auto it = object.find(key);
    return it != object.end() && it->is_string() ? it->get<std::string>() : std::string();
}

const json &objectIn(const json &parent, const char *key)
{
    static const json empty = json::object();
    const auto it = parent.find(key);
    return it != parent.end() && it->is_object() ? *it : empty;
}

const json &arrayIn(const json &parent, const char *key)
{
    static const json empty = json::array();
    const auto it = parent.find(key);
    return it != parent.end() && it->is_array() ? *it : empty;
}

std::vector<std::string> stringsIn(const json &parent, const char *key)
{
    std::vector<std::string> out;
    for (const json &item : arrayIn(parent, key)) {
        if (item.is_string())
            out.push_back(item.get<std::string>());
    }
    return out;
}

// The web app counts attempts for lessons only; checkpoint attempts are the desktop app's, and it ignores them.
json scoreJson(const Score &score)
{
    return {{"date", toIso(score.date)}, {"score", score.score}, {"best", score.best}, {"attempts", score.attempts}};
}

Score scoreIn(const json &value)
{
    return {dateIn(value.at("date")), numberIn(value, "score"), numberIn(value, "best"), intIn(value, "attempts", 1)};
}

std::string feelText(const std::optional<Feel> &feel)
{
    return feel ? std::string(feelId(*feel)) : std::string();
}

} // namespace

std::string writeBackup(const Backup &backup, const Course &course, std::string_view exportedAt)
{
    const LearnerState &s = backup.state;
    const auto unitId = [&course](int index) {
        return index >= 0 && index < static_cast<int>(course.units.size()) ? course.units[static_cast<std::size_t>(index)].id
                                                                            : std::string();
    };

    json plan{{"days", s.plan.days}, {"start", toIso(s.plan.start)}, {"studyDays", json::array()},
              {"skipHolidays", s.plan.skipHolidays}, {"daysOff", json::array()}};
    for (const int day : s.plan.studyDays)
        plan["studyDays"].push_back(day);
    for (const Date day : s.plan.daysOff)
        plan["daysOff"].push_back(toIso(day));

    json done = json::array();
    for (const DoneDay &day : s.done)
        done.push_back({{"n", day.n}, {"date", toIso(day.date)}});
    json tasks = json::object();
    for (const auto &[n, byTask] : s.tasks) {
        json day = json::object();
        for (const auto &[task, date] : byTask)
            day[std::string(taskId(task))] = toIso(date);
        tasks[std::to_string(n)] = day;
    }
    json units = json::object();
    for (const auto &[index, score] : s.units) {
        if (const std::string id = unitId(index); !id.empty())
            units[id] = scoreJson(score);
    }
    json checkpoints = json::object();
    for (const auto &[phase, score] : s.checkpoints)
        checkpoints[std::to_string(phase)] = scoreJson(score);

    std::vector<std::pair<const std::string *, const Card *>> deck;
    for (const auto &[id, card] : s.cards)
        deck.emplace_back(&id, &card);
    std::ranges::sort(deck, [](const auto &a, const auto &b) { return a.second->seq < b.second->seq; });
    json cards = json::object();
    for (const auto &[id, card] : deck) {
        cards[*id] = {{"box", card->box}, {"due", toIso(card->due)}, {"reps", card->reps}, {"lapses", card->lapses},
                      {"added", toIso(card->added)}, {"last", card->last ? json(toIso(*card->last)) : json(nullptr)},
                      {"u", unitId(card->unit)}};
    }

    json activity = json::object();
    for (const auto &[date, a] : s.activity) {
        json ex = json::object();
        for (const auto &[skill, tally] : a.answers)
            ex[skill] = json::array({tally.correct, tally.total});
        activity[toIso(date)] = {{"xp", a.xp},        {"sec", a.seconds},      {"input", a.inputMinutes},
                                 {"island", a.islandMinutes}, {"talk", a.talkMinutes}, {"speak", a.speak},
                                 {"ex", ex}};
    }
    json frozen = json::array();
    for (const Date day : s.frozen)
        frozen.push_back(toIso(day));

    // The web app keeps an id on each entry; the position is enough to make one.
    json immersion = json::array();
    for (std::size_t i = 0; i < s.input.size(); ++i) {
        const InputEntry &e = s.input[i];
        json entry{{"id", "d" + std::to_string(i)}, {"date", toIso(e.date)}, {"min", e.minutes}, {"kind", e.kind},
                   {"source", e.source}};
        if (e.imported)
            entry["imported"] = true;  // the web app keeps the key and gives no XP back on removal either
        immersion.push_back(std::move(entry));
    }
    json exchanges = json::array();
    for (std::size_t i = 0; i < s.exchanges.size(); ++i) {
        const Exchange &e = s.exchanges[i];
        exchanges.push_back({{"id", "x" + std::to_string(i)}, {"date", toIso(e.date)}, {"min", e.minutes},
                             {"partner", e.partner}, {"note", e.note}});
    }
    json missions = json::object();
    for (const auto &[id, date] : s.missions)
        missions[id] = toIso(date);
    json achievements = json::object();
    for (const auto &[id, date] : s.achievements)
        achievements[id] = toIso(date);

    json reflections = json::array();
    for (const Reflection &r : s.reflections) {
        reflections.push_back({{"date", toIso(r.date)}, {"activity", r.activity}, {"offered", r.offered},
                               {"liked", r.liked}, {"feel", feelText(r.feel)}, {"note", r.note}});
    }
    const BackupProfile &p = backup.profile;
    // The web app writes its placement result for the top level.
    const json level = p.level.empty() ? json(nullptr) : json(p.level == "A2+" ? "A2+ / B1\xE2\x88\x92" : p.level);
    json desktop{{"reason", p.reason},   {"goals", p.goals},          {"streakEmoji", p.streakEmoji},
                 {"sessions", s.sessions}, {"reflectedAt", s.reflectedAt}, {"reflections", reflections}};

    json state{
        {"version", 1},
        {"createdAt", toIso(s.plan.start)},
        {"onboarded", true},
        {"profile", {{"name", p.name}, {"level", level}}},
        {"plan", plan},
        {"done", done},
        {"tasks", tasks},
        {"units", units},
        {"checkpoints", checkpoints},
        {"cards", cards},
        {"activity", activity},
        {"streak", {{"freezes", s.freezes}, {"frozen", frozen}, {"longest", s.longest}}},
        {"immersion", immersion},
        {"exchanges", exchanges},
        {"missions", missions},
        {"achievements", achievements},
        {"writing", json::array()},
        {"desktop", desktop},
    };
    json file{{"app", "LearnQuick"}, {"exported", std::string(exportedAt)}, {"state", state}};
    return file.dump(1) + "\n";
}

std::optional<Backup> readBackup(std::string_view text, const Course &course, std::string *error)
{
    const auto fail = [error](const std::string &why) -> std::optional<Backup> {
        if (error)
            *error = why;
        return std::nullopt;
    };
    if (text.size() > kMaxBackupBytes)
        return fail("This file is too large to be a LearnQuick backup.");
    json file;
    try {
        file = json::parse(text);
    } catch (const json::exception &) {
        return fail("This file is not a LearnQuick backup: it is not JSON.");
    }
    // As the web app's parseBackup(): {state} or the state itself, from this app and no other, in the one version
    // there is, with a profile, a plan that starts on a date and the activity.
    if (!file.is_object() || (file.contains("app") && file.at("app") != "LearnQuick"))
        return fail("This file is not a LearnQuick backup.");
    const json &s = file.contains("state") ? file.at("state") : file;
    const auto holds = [&s](const char *key) { return s.contains(key) && s.at(key).is_object(); };
    if (!s.is_object() || !holds("profile") || !holds("plan") || !holds("activity")
        || !s.at("plan").contains("start") || !s.at("plan").at("start").is_string()
        || (s.contains("version") && !(s.at("version").is_number() && s.at("version").get<double>() == 1.0)))
        return fail("This file is not a supported LearnQuick backup.");

    std::map<std::string, int, std::less<>> unitIndex;
    for (std::size_t i = 0; i < course.units.size(); ++i)
        unitIndex.emplace(course.units[i].id, static_cast<int>(i));

    Backup backup;
    LearnerState &state = backup.state;
    try {
        // A part that is missing reads as empty; one of the wrong kind is damage, and reading it as empty would
        // restore a learner without it.
        for (const char *list : {"done", "immersion", "exchanges", "writing"}) {
            if (s.contains(list) && !s.at(list).is_array())
                throw BadFile{"\"" + std::string(list) + "\" is not a list"};
        }
        for (const char *group : {"tasks", "units", "checkpoints", "cards", "streak"}) {
            if (s.contains(group) && !s.at(group).is_object())
                throw BadFile{"\"" + std::string(group) + "\" is not a group of entries"};
        }

        const json &profile = objectIn(s, "profile");
        backup.profile.name = textIn(profile, "name");
        backup.profile.level = textIn(profile, "level");
        if (backup.profile.level.starts_with("A2+"))
            backup.profile.level = "A2+";  // the web app's "A2+ / B1−"
        else if (backup.profile.level != "A1" && backup.profile.level != "A2")
            backup.profile.level.clear();

        const json &plan = objectIn(s, "plan");
        const int days = intIn(plan, "days", 60);
        state.plan.days = days == 30 || days == 90 ? days : 60;
        state.plan.start = dateIn(plan.at("start"));
        if (plan.contains("studyDays")) {
            state.plan.studyDays.clear();
            for (const json &day : arrayIn(plan, "studyDays")) {
                if (day.is_number() && day.get<double>() >= 0 && day.get<double>() <= 6)
                    state.plan.studyDays.insert(static_cast<int>(day.get<double>()));
            }
            if (state.plan.studyDays.empty())
                state.plan.studyDays = {1, 2, 3, 4, 5};
        }
        if (const auto it = plan.find("skipHolidays"); it != plan.end() && it->is_boolean())
            state.plan.skipHolidays = it->get<bool>();
        for (const json &day : arrayIn(plan, "daysOff"))
            state.plan.daysOff.insert(dateIn(day));

        for (const json &day : arrayIn(s, "done"))
            state.done.push_back({intIn(day, "n"), dateIn(day.at("date"))});
        for (const auto &[n, byTask] : objectIn(s, "tasks").items()) {
            if (!byTask.is_object())
                continue;
            for (const auto &[task, date] : byTask.items()) {
                if (const auto known = taskFromId(task))
                    state.tasks[std::stoi(n)][*known] = dateIn(date);
            }
        }
        for (const auto &[id, score] : objectIn(s, "units").items()) {
            if (const auto it = unitIndex.find(id); it != unitIndex.end())
                state.units[it->second] = scoreIn(score);
        }
        for (const auto &[phase, score] : objectIn(s, "checkpoints").items())
            state.checkpoints[std::stoi(phase)] = scoreIn(score);

        int seq = 0;
        for (const auto &[id, c] : objectIn(s, "cards").items()) {
            const auto unit = unitIndex.find(textIn(c, "u"));
            if (unit == unitIndex.end())
                continue;
            Card card;
            card.unit = unit->second;
            card.box = std::clamp(intIn(c, "box", 1), 0, kMaxBox);
            card.due = dateIn(c.at("due"));
            card.reps = intIn(c, "reps");
            card.lapses = intIn(c, "lapses");
            card.added = dateIn(c.at("added"));
            card.last = optionalDate(c, "last");
            card.seq = seq++;
            state.cards.emplace(id, card);
        }

        for (const auto &[day, a] : objectIn(s, "activity").items()) {
            Activity &activity = state.activity[dateIn(json(day))];
            activity.xp = intIn(a, "xp");
            activity.seconds = intIn(a, "sec");
            activity.inputMinutes = intIn(a, "input");
            activity.islandMinutes = intIn(a, "island");
            activity.talkMinutes = intIn(a, "talk");
            activity.speak = intIn(a, "speak");
            for (const auto &[skill, tally] : objectIn(a, "ex").items()) {
                if (tally.is_array() && tally.size() == 2 && tally[0].is_number() && tally[1].is_number())
                    activity.answers[skill] = {countOf(tally[0]), countOf(tally[1])};
            }
        }
        const json &streak = objectIn(s, "streak");
        state.freezes = intIn(streak, "freezes");
        state.longest = intIn(streak, "longest");
        for (const json &day : arrayIn(streak, "frozen"))
            state.frozen.insert(dateIn(day));

        // Entries get the limits the screens give them, however the file was written.
        const auto logText = [](const json &e, const char *key) { return cutText(textIn(e, key), kMaxLogText); };
        for (const json &e : arrayIn(s, "immersion")) {
            const int minutes = intIn(e, "min");
            if (minutes > 0)
                state.input.push_back({dateIn(e.at("date")), std::min(minutes, kMaxLoggedMinutes), cutText(textIn(e, "kind"), 20),
                                       logText(e, "source"),
                                       e.contains("imported") && e.at("imported").is_boolean() && e.at("imported").get<bool>()});
        }
        for (const json &e : arrayIn(s, "exchanges")) {
            const int minutes = intIn(e, "min");
            if (minutes > 0)
                state.exchanges.push_back({dateIn(e.at("date")), std::min(minutes, kMaxLoggedMinutes), logText(e, "partner"),
                                           logText(e, "note")});
        }
        for (const auto &[id, date] : objectIn(s, "missions").items()) {
            if (!date.is_null() && !(date.is_boolean() && !date.get<bool>()))
                state.missions.emplace(id, date.is_string() ? dateIn(date) : state.plan.start);
        }
        for (const auto &[id, date] : objectIn(s, "achievements").items()) {
            if (date.is_string())
                state.achievements.emplace(id, dateIn(date));
        }

        const json &desktop = objectIn(s, "desktop");
        backup.profile.desktop = s.contains("desktop");
        backup.profile.reason = textIn(desktop, "reason");
        backup.profile.goals = stringsIn(desktop, "goals");
        backup.profile.streakEmoji = textIn(desktop, "streakEmoji");
        state.sessions = intIn(desktop, "sessions");
        state.reflectedAt = std::min(intIn(desktop, "reflectedAt"), state.sessions);
        for (const json &r : arrayIn(desktop, "reflections")) {
            state.reflections.push_back({dateIn(r.at("date")), textIn(r, "activity"), stringsIn(r, "offered"),
                                         stringsIn(r, "liked"), feelFromId(textIn(r, "feel")), textIn(r, "note")});
        }
    } catch (const BadFile &bad) {
        return fail("This backup has a damaged entry: " + bad.why + ".");
    } catch (const std::exception &) {
        return fail("This backup has a damaged entry.");
    }
    return backup;
}

} // namespace lq
