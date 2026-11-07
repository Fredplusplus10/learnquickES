#include "lq/database.h"
#include "lq/learner.h"
#include "lq/profile.h"
#include "lq/settings.h"

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTimeZone>

#include <initializer_list>
#include <utility>

namespace {

lq::Profile sampleProfile()
{
    lq::Profile profile;
    profile.name = QStringLiteral("Cory");
    profile.reason = QStringLiteral("I want to talk with my neighbors and coworkers in Spanish without switching to English.");
    profile.goals = {QStringLiteral("work"), QStringLiteral("community")};
    profile.level = QStringLiteral("A2");
    profile.planDays = 60;
    profile.planStart = QDate(2026, 11, 2);
    profile.createdAt = QDateTime(QDate(2026, 11, 2), QTime(9, 30, 15, 250), QTimeZone::UTC);
    profile.streakEmoji = QStringLiteral("🌮");
    return profile;
}

lq::Date day(int y, unsigned m, unsigned d)
{
    return lq::Date(std::chrono::year(y) / std::chrono::month(m) / std::chrono::day(d));
}

lq::Plan samplePlan()
{
    lq::Plan plan;
    plan.days = 60;
    plan.start = day(2026, 11, 2);
    return plan;
}

// A learner a few days in, with something in every table.
lq::LearnerState sampleLearner()
{
    lq::LearnerState state = lq::defaultState(day(2026, 11, 2));
    state.plan = samplePlan();
    state.done = {{1, day(2026, 11, 2)}, {2, day(2026, 11, 3)}};
    state.tasks[3] = {{lq::Task::Review, day(2026, 11, 4)}, {lq::Task::Verbs, day(2026, 11, 4)}};
    state.units[0] = {day(2026, 11, 2), 0.8, 0.9, 2};
    state.checkpoints[1] = {day(2026, 11, 3), 0.75, 0.75, 1};
    lq::Activity &activity = state.activity[day(2026, 11, 4)];
    activity.xp = 120;
    activity.seconds = 1500;
    activity.inputMinutes = 20;
    activity.islandMinutes = 5;
    activity.speak = 3;
    activity.talkMinutes = 30;
    activity.answers["verbs"] = {7, 10};
    activity.answers["review"] = {12, 14};
    lq::Card hola = lq::newCard(day(2026, 11, 2));
    hola.unit = 0;
    hola = lq::gradeCard(hola, lq::Grade::Good, day(2026, 11, 3));
    lq::Card adios = lq::newCard(day(2026, 11, 2));
    adios.unit = 0;
    adios.seq = 1;
    state.cards = {{"hola", hola}, {"adios", adios}};
    state.freezes = 1;
    state.frozen = {day(2026, 11, 5)};
    state.longest = 3;
    state.sessions = 7;
    state.reflectedAt = 6;
    state.reflections = {
        {day(2026, 11, 3), "mixed", {"choice", "listen", "order"}, {"listen", "order"}, lq::Feel::Hard, "Me gustó escuchar."},
        {day(2026, 11, 4), "coach", {"conj", "speak"}, {}, std::nullopt, ""},
    };
    state.input = {{day(2026, 11, 3), 15, "podcast", "Españolistos"}, {day(2026, 11, 4), 20, "series", ""},
                   {day(2026, 11, 4), 5, "island", ""}, {day(2026, 11, 1), 42, "series", "Netflix · Élite: Temporada 1: Bienvenidos", true}};
    state.exchanges = {{day(2026, 11, 4), 30, "Lucía (Tandem)", "Hablamos del fin de semana."}};
    state.missions = {{"phone", day(2026, 11, 3)}, {"youtube", day(2026, 11, 4)}};
    state.achievements = {{"first_lesson", day(2026, 11, 2)}, {"exchange_1", day(2026, 11, 4)}};
    return state;
}

// Undoes every migration after `version`, newest first: what an older app left behind.
void rewindSchema(const QSqlDatabase &db, int version)
{
    QSqlQuery query(db);
    const auto drop = [&query](std::initializer_list<const char *> tables) {
        for (const char *table : tables)
            REQUIRE(query.exec(QStringLiteral("DROP TABLE %1").arg(QLatin1StringView(table))));
    };
    if (version < 10)
        drop({"assessment_result"});  // its index and triggers go with it
    if (version < 9 && version >= 6)
        REQUIRE(query.exec(QStringLiteral("ALTER TABLE input_log DROP COLUMN imported")));
    if (version < 8)
        drop({"certificate_award"});  // its triggers go with it
    if (version < 6) {
        drop({"input_log", "exchange_log", "mission", "achievement"});
        REQUIRE(query.exec(QStringLiteral("ALTER TABLE activity DROP COLUMN talk_minutes")));
        REQUIRE(query.exec(QStringLiteral("ALTER TABLE profile DROP COLUMN skip_holidays")));
        REQUIRE(query.exec(QStringLiteral("ALTER TABLE profile DROP COLUMN days_off")));
    }
    if (version < 5)
        drop({"setting"});
    if (version < 4)
        REQUIRE(query.exec(QStringLiteral("ALTER TABLE profile DROP COLUMN streak_emoji")));
    if (version < 3)
        drop({"reflection", "session_count"});
    if (version < 2)
        drop({"plan_day", "task_done", "unit_result", "checkpoint_result", "activity", "answer_tally", "card", "streak",
              "frozen_day"});
    REQUIRE(query.exec(QStringLiteral("PRAGMA user_version = %1").arg(version)));
}

} // namespace

TEST_CASE("a new database is created at the latest schema, without a profile", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    lq::Database db;
    REQUIRE(db.open(dir.filePath(QStringLiteral("learnquick.sqlite3"))));
    CHECK(db.schemaVersion() == lq::Database::latestSchemaVersion());
    CHECK_FALSE(lq::loadProfile(db.connection()).has_value());
}

TEST_CASE("the profile survives closing and reopening the database", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    lq::Profile profile = sampleProfile();
    profile.skipHolidays = false;
    profile.daysOff = {QDate(2026, 11, 27), QDate(2026, 12, 24)};
    {
        lq::Database db;
        REQUIRE(db.open(path));
        QString error;
        REQUIRE(lq::saveProfile(db.connection(), profile, &error));
        CHECK(error.isEmpty());
    }
    lq::Database db;
    REQUIRE(db.open(path));
    const auto loaded = lq::loadProfile(db.connection());
    REQUIRE(loaded.has_value());
    CHECK(*loaded == profile);
}

TEST_CASE("saving again replaces the single profile row", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    lq::Database db;
    REQUIRE(db.open(dir.filePath(QStringLiteral("learnquick.sqlite3"))));
    lq::Profile profile = sampleProfile();
    REQUIRE(lq::saveProfile(db.connection(), profile));
    profile.reason = QStringLiteral("My partner's family speaks Spanish at every dinner and I want to join in.");
    REQUIRE(lq::saveProfile(db.connection(), profile));
    QSqlQuery count(db.connection());
    REQUIRE(count.exec(QStringLiteral("SELECT COUNT(*) FROM profile")));
    REQUIRE(count.next());
    CHECK(count.value(0).toInt() == 1);
    CHECK(lq::loadProfile(db.connection())->reason == profile.reason);
}

TEST_CASE("a database written by a newer version of the app is refused", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    {
        lq::Database db;
        REQUIRE(db.open(path));
        REQUIRE(QSqlQuery(db.connection()).exec(QStringLiteral("PRAGMA user_version = 99")));
    }
    lq::Database db;
    CHECK_FALSE(db.open(path));
    CHECK(db.error().contains(QStringLiteral("99")));
}

TEST_CASE("profile validation says what is missing", "[storage]")
{
    CHECK(lq::validateProfile(sampleProfile()).isEmpty());

    lq::Profile profile = sampleProfile();
    profile.name = QStringLiteral("   ");
    CHECK(lq::validateProfile(profile).contains(QStringLiteral("name")));

    profile = sampleProfile();
    profile.reason = QStringLiteral("travel");
    CHECK(lq::validateProfile(profile).contains(QStringLiteral("why")));

    profile = sampleProfile();
    profile.level = QStringLiteral("C2");
    CHECK(lq::validateProfile(profile).contains(QStringLiteral("level")));

    profile = sampleProfile();
    profile.planDays = 45;
    CHECK(lq::validateProfile(profile).contains(QStringLiteral("plan")));
}

TEST_CASE("the learner's progress survives closing and reopening the database", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    const lq::LearnerState saved = sampleLearner();
    {
        lq::Database db;
        REQUIRE(db.open(path));
        QString error;
        REQUIRE(lq::saveLearner(db.connection(), saved, &error));
        CHECK(error.isEmpty());
    }
    lq::Database db;
    REQUIRE(db.open(path));
    QString error;
    const auto loaded = lq::loadLearner(db.connection(), samplePlan(), &error);
    REQUIRE(loaded.has_value());
    CHECK(error.isEmpty());

    REQUIRE(loaded->done.size() == saved.done.size());
    for (std::size_t i = 0; i < saved.done.size(); ++i) {
        CHECK(loaded->done[i].n == saved.done[i].n);
        CHECK(loaded->done[i].date == saved.done[i].date);
    }
    CHECK(loaded->tasks == saved.tasks);
    REQUIRE(loaded->units.size() == 1);
    CHECK(loaded->units.at(0).best == 0.9);
    CHECK(loaded->units.at(0).score == 0.8);
    CHECK(loaded->units.at(0).attempts == 2);
    CHECK(loaded->checkpoints.at(1).best == 0.75);

    const lq::Activity &a = loaded->activity.at(day(2026, 11, 4));
    CHECK(a.xp == 120);
    CHECK(a.seconds == 1500);
    CHECK(a.inputMinutes == 20);
    CHECK(a.islandMinutes == 5);
    CHECK(a.speak == 3);
    CHECK(a.talkMinutes == 30);
    CHECK(a.answers.at("verbs").correct == 7);
    CHECK(a.answers.at("review").total == 14);

    REQUIRE(loaded->cards.size() == 2);
    for (const auto &[id, card] : saved.cards) {
        CAPTURE(id);
        const lq::Card &got = loaded->cards.at(id);
        CHECK(got.unit == card.unit);
        CHECK(got.box == card.box);
        CHECK(got.due == card.due);
        CHECK(got.reps == card.reps);
        CHECK(got.lapses == card.lapses);
        CHECK(got.added == card.added);
        CHECK(got.last == card.last);
        CHECK(got.seq == card.seq);
    }
    CHECK(loaded->freezes == 1);
    CHECK(loaded->frozen == saved.frozen);
    CHECK(loaded->longest == 3);
    CHECK(loaded->plan.start == samplePlan().start);

    CHECK(loaded->sessions == 7);
    CHECK(loaded->reflectedAt == 6);
    REQUIRE(loaded->reflections.size() == 2);
    for (std::size_t i = 0; i < saved.reflections.size(); ++i) {
        CAPTURE(i);
        const lq::Reflection &want = saved.reflections[i];
        const lq::Reflection &got = loaded->reflections[i];
        CHECK(got.date == want.date);
        CHECK(got.activity == want.activity);
        CHECK(got.offered == want.offered);
        CHECK(got.liked == want.liked);
        CHECK(got.feel == want.feel);
        CHECK(got.note == want.note);
    }

    REQUIRE(loaded->input.size() == saved.input.size());
    for (std::size_t i = 0; i < saved.input.size(); ++i) {
        CAPTURE(i);
        CHECK(loaded->input[i].date == saved.input[i].date);
        CHECK(loaded->input[i].minutes == saved.input[i].minutes);
        CHECK(loaded->input[i].kind == saved.input[i].kind);
        CHECK(loaded->input[i].source == saved.input[i].source);
    }
    REQUIRE(loaded->exchanges.size() == 1);
    CHECK(loaded->exchanges[0].date == day(2026, 11, 4));
    CHECK(loaded->exchanges[0].minutes == 30);
    CHECK(loaded->exchanges[0].partner == "Lucía (Tandem)");
    CHECK(loaded->exchanges[0].note == "Hablamos del fin de semana.");
    CHECK(loaded->missions == saved.missions);
    CHECK(loaded->achievements == saved.achievements);
}

TEST_CASE("saving the learner again replaces the previous snapshot", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    lq::Database db;
    REQUIRE(db.open(dir.filePath(QStringLiteral("learnquick.sqlite3"))));
    lq::LearnerState state = sampleLearner();
    REQUIRE(lq::saveLearner(db.connection(), state));
    state.cards.erase("adios");
    state.frozen.clear();
    REQUIRE(lq::saveLearner(db.connection(), state));
    const auto loaded = lq::loadLearner(db.connection(), samplePlan());
    REQUIRE(loaded.has_value());
    CHECK(loaded->cards.size() == 1);
    CHECK(loaded->frozen.empty());
}

// Reads a database file without migrating it: its schema version and its profile's name.
static std::pair<int, QString> peek(const QString &path)
{
    std::pair<int, QString> found{-1, {}};
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("peek"));
        db.setDatabaseName(path);
        db.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
        if (db.open()) {
            QSqlQuery q(db);
            if (q.exec(QStringLiteral("PRAGMA user_version")) && q.next())
                found.first = q.value(0).toInt();
            if (q.exec(QStringLiteral("SELECT name FROM profile")) && q.next())
                found.second = q.value(0).toString();
        }
        db.close();
    }
    QSqlDatabase::removeDatabase(QStringLiteral("peek"));
    return found;
}

TEST_CASE("data is copied beside the file before a migration, and only then", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    const QString copy = path + QStringLiteral(".v1.bak");

    // A new file has nothing to copy, and one already at the latest schema needs no migration.
    {
        lq::Database db;
        REQUIRE(db.open(path));
        CHECK(db.migrationBackup().isEmpty());
        REQUIRE(lq::saveProfile(db.connection(), sampleProfile()));
    }
    {
        lq::Database db;
        REQUIRE(db.open(path));
        CHECK(db.migrationBackup().isEmpty());
        rewindSchema(db.connection(), 1);  // as version 0.2 left it
    }
    CHECK_FALSE(QFileInfo::exists(copy));

    // Data at schema 1: copied as it was, then migrated.
    {
        lq::Database db;
        REQUIRE(db.open(path));
        CHECK(db.migrationBackup() == copy);
        CHECK(db.schemaVersion() == lq::Database::latestSchemaVersion());
    }
    CHECK(peek(copy) == std::pair<int, QString>{1, QStringLiteral("Cory")});
    CHECK(peek(path).first == lq::Database::latestSchemaVersion());
}

TEST_CASE("when the copy cannot be made, nothing is migrated", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    {
        lq::Database db;
        REQUIRE(db.open(path));
        REQUIRE(lq::saveProfile(db.connection(), sampleProfile()));
        rewindSchema(db.connection(), 1);
    }
    // A folder where the copy would go: the copy fails.
    REQUIRE(QDir(dir.path()).mkdir(QStringLiteral("learnquick.sqlite3.v1.bak")));
    {
        lq::Database db;
        CHECK_FALSE(db.open(path));
        CHECK(db.error().contains(u"copy could not be saved"));
    }
    CHECK(peek(path) == std::pair<int, QString>{1, QStringLiteral("Cory")});

    // Once the way is clear, the next start copies and migrates.
    REQUIRE(QDir(dir.path()).rmdir(QStringLiteral("learnquick.sqlite3.v1.bak")));
    lq::Database db;
    REQUIRE(db.open(path));
    CHECK(QFileInfo(db.migrationBackup()).isFile());
    CHECK(db.schemaVersion() == lq::Database::latestSchemaVersion());
}

TEST_CASE("a phase 2 database gains the progress tables and keeps the profile", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    {
        // Rebuild what version 0.2 left behind: the profile table at schema 1.
        lq::Database db;
        REQUIRE(db.open(path));
        REQUIRE(lq::saveProfile(db.connection(), sampleProfile()));
        rewindSchema(db.connection(), 1);
    }
    lq::Database db;
    REQUIRE(db.open(path));
    CHECK(db.schemaVersion() == lq::Database::latestSchemaVersion());
    const auto profile = lq::loadProfile(db.connection());
    REQUIRE(profile.has_value());
    lq::Profile expected = sampleProfile();
    expected.streakEmoji.clear();
    CHECK(*profile == expected);
    const auto learner = lq::loadLearner(db.connection(), samplePlan());
    REQUIRE(learner.has_value());
    CHECK(learner->done.empty());
    CHECK(learner->cards.empty());
}

TEST_CASE("a phase 3 database gains the coach tables and keeps the progress", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    lq::LearnerState before = sampleLearner();
    before.reflections.clear();
    before.input.clear();
    before.exchanges.clear();
    before.missions.clear();
    before.achievements.clear();
    before.activity.begin()->second.talkMinutes = 0;
    {
        // What version 0.3.1 left behind: progress at schema 2, no reflections yet.
        lq::Database db;
        REQUIRE(db.open(path));
        REQUIRE(lq::saveLearner(db.connection(), before));
        rewindSchema(db.connection(), 2);
    }
    lq::Database db;
    REQUIRE(db.open(path));
    CHECK(db.schemaVersion() == lq::Database::latestSchemaVersion());
    const auto learner = lq::loadLearner(db.connection(), samplePlan());
    REQUIRE(learner.has_value());
    CHECK(learner->done.size() == before.done.size());
    CHECK(learner->cards.size() == before.cards.size());
    CHECK(learner->reflections.empty());
    CHECK(learner->sessions == 0);
    CHECK_FALSE(lq::reflectionDue(*learner));
}

TEST_CASE("a 0.3.4 database gains the streak emoji, set to the app's own flame", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    {
        lq::Database db;
        REQUIRE(db.open(path));
        REQUIRE(lq::saveProfile(db.connection(), sampleProfile()));
        rewindSchema(db.connection(), 3);
    }
    lq::Database db;
    REQUIRE(db.open(path));
    CHECK(db.schemaVersion() == lq::Database::latestSchemaVersion());
    const auto profile = lq::loadProfile(db.connection());
    REQUIRE(profile.has_value());
    CHECK(profile->name == sampleProfile().name);
    CHECK(profile->streakEmoji.isEmpty());
}

TEST_CASE("the streak takes any single emoji, whole", "[storage]")
{
    // Plain, with a variation selector, a flag, a skin tone, a ZWJ family, a keycap.
    const QStringList emojis{QStringLiteral("🔥"), QStringLiteral("❤️"), QStringLiteral("🇲🇽"),
                             QStringLiteral("👍🏽"), QStringLiteral("👨‍👩‍👧‍👦"), QStringLiteral("1️⃣")};
    for (const QString &emoji : emojis) {
        INFO(emoji.toStdString());
        CHECK(lq::firstEmoji(emoji) == emoji);
        CHECK(lq::firstEmoji(QStringLiteral("  ") + emoji + QStringLiteral(" and more")) == emoji);
    }
    CHECK(lq::firstEmoji(QStringLiteral("🌮🌮")) == QStringLiteral("🌮"));
    const QStringList notEmoji{QString(), QStringLiteral("   "), QStringLiteral("a🔥"), QStringLiteral("ñ"),
                               QStringLiteral("1"), QStringLiteral("?")};
    for (const QString &text : notEmoji)
        CHECK(lq::firstEmoji(text).isEmpty());

    lq::Profile profile = sampleProfile();
    profile.streakEmoji = QStringLiteral("🔥🔥");
    CHECK(lq::validateProfile(profile).contains(QStringLiteral("emoji")));
    profile.streakEmoji = QStringLiteral("abc");
    CHECK(lq::validateProfile(profile).contains(QStringLiteral("emoji")));
    profile.streakEmoji.clear();
    CHECK(lq::validateProfile(profile).isEmpty());
}

TEST_CASE("a 0.3.5 database gains settings, read as the recommended ones until changed", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    {
        lq::Database db;
        REQUIRE(db.open(path));
        REQUIRE(lq::saveProfile(db.connection(), sampleProfile()));
        rewindSchema(db.connection(), 4);
    }
    lq::Database db;
    REQUIRE(db.open(path));
    CHECK(db.schemaVersion() == lq::Database::latestSchemaVersion());
    CHECK(lq::loadProfile(db.connection()) == sampleProfile());
    CHECK(lq::loadSettings(db.connection()) == lq::Settings{});
}

TEST_CASE("a 0.3.6 database keeps its progress and starts the immersion log empty", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    lq::LearnerState before = sampleLearner();
    before.input.clear();
    before.exchanges.clear();
    before.missions.clear();
    before.achievements.clear();
    before.activity.begin()->second.talkMinutes = 0;
    {
        lq::Database db;
        REQUIRE(db.open(path));
        REQUIRE(lq::saveLearner(db.connection(), before));
        rewindSchema(db.connection(), 5);
    }
    lq::Database db;
    REQUIRE(db.open(path));
    CHECK(db.schemaVersion() == lq::Database::latestSchemaVersion());
    const auto learner = lq::loadLearner(db.connection(), samplePlan());
    REQUIRE(learner.has_value());
    CHECK(learner->activity.at(day(2026, 11, 4)).inputMinutes == 20);  // listening logged before 0.4 stays
    CHECK(learner->activity.at(day(2026, 11, 4)).talkMinutes == 0);
    CHECK(learner->input.empty());
    CHECK(learner->exchanges.empty());
    CHECK(learner->achievements.empty());
    CHECK(learner->reflections.size() == before.reflections.size());
}

TEST_CASE("a 0.4.0 database forgets its voice and paces, and keeps the rest", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    {
        lq::Database db;
        REQUIRE(db.open(path));
        REQUIRE(lq::saveProfile(db.connection(), sampleProfile()));
        QSqlQuery query(db.connection());
        REQUIRE(query.exec(QStringLiteral("INSERT INTO setting (key, value) VALUES ('theme', 'dark'), "
                                          "('voice', 'M1'), ('pace', '1.60'), ('slow_pace', '2.20')")));
        rewindSchema(db.connection(), 6);
    }
    lq::Database db;
    REQUIRE(db.open(path));
    CHECK(db.schemaVersion() == lq::Database::latestSchemaVersion());
    CHECK(lq::loadProfile(db.connection()) == sampleProfile());
    lq::Settings expected;
    expected.theme = QStringLiteral("dark");
    CHECK(lq::loadSettings(db.connection()) == expected);
}

TEST_CASE("settings survive reopening the database", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    lq::Settings settings;
    settings.theme = QStringLiteral("dark");
    settings.voice = QStringLiteral("M5");
    settings.pace = 1.35;
    settings.slowPace = 1.7;
    {
        lq::Database db;
        REQUIRE(db.open(path));
        CHECK(lq::loadSettings(db.connection()) == lq::Settings{});
        REQUIRE(lq::saveSettings(db.connection(), settings));
    }
    lq::Database db;
    REQUIRE(db.open(path));
    CHECK(lq::loadSettings(db.connection()) == settings);
}

TEST_CASE("settings out of range read as the nearest ones the screen offers", "[storage]")
{
    lq::Settings odd;
    odd.theme = QStringLiteral("sepia");
    odd.voice = QStringLiteral("F9");
    odd.pace = 0.5;
    odd.slowPace = 9.0;
    lq::Settings clean = lq::normalized(odd);
    CHECK(clean.theme == QStringLiteral("system"));
    CHECK(clean.voice == lq::Settings{}.voice);
    CHECK(clean.pace == lq::kMinPace);
    CHECK(clean.slowPace == lq::kMaxSlowPace);

    // Paces move in steps of 0.05, and slow stays slower than normal.
    odd = lq::Settings{};
    odd.pace = 1.273;
    odd.slowPace = 1.3;
    clean = lq::normalized(odd);
    CHECK(clean.pace == 1.25);
    CHECK(clean.slowPace == 1.4);

    // Something stored by hand, or by a newer version, never breaks loading.
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    lq::Database db;
    REQUIRE(db.open(dir.filePath(QStringLiteral("learnquick.sqlite3"))));
    QSqlQuery query(db.connection());
    REQUIRE(query.exec(QStringLiteral("INSERT INTO setting (key, value) VALUES ('pace', 'fast'), "
                                      "('theme', 'dark'), ('font', 'large')")));
    const lq::Settings loaded = lq::loadSettings(db.connection());
    CHECK(loaded.pace == lq::kDefaultPace);
    CHECK(loaded.theme == QStringLiteral("dark"));
}

TEST_CASE("a restored backup replaces the profile and the progress together", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    lq::Database db;
    REQUIRE(db.open(dir.filePath(QStringLiteral("learnquick.sqlite3"))));
    REQUIRE(lq::saveProfile(db.connection(), sampleProfile()));
    REQUIRE(lq::saveLearner(db.connection(), lq::defaultState(day(2026, 11, 2))));

    lq::Profile restored = sampleProfile();
    restored.name = QStringLiteral("Ana");
    restored.planDays = 90;
    const lq::LearnerState progress = sampleLearner();
    QString error;
    REQUIRE(lq::restoreLearner(db.connection(), restored, progress, &error));
    CHECK(error.isEmpty());
    CHECK(lq::loadProfile(db.connection()) == restored);
    const auto loaded = lq::loadLearner(db.connection(), samplePlan());
    REQUIRE(loaded);
    CHECK(*loaded == progress);
}

TEST_CASE("a restore that fails halfway changes neither the profile nor the progress", "[storage]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    lq::Database db;
    REQUIRE(db.open(dir.filePath(QStringLiteral("learnquick.sqlite3"))));
    REQUIRE(lq::saveProfile(db.connection(), sampleProfile()));
    const lq::LearnerState before = sampleLearner();
    REQUIRE(lq::saveLearner(db.connection(), before));
    // Achievements are written last: failing there comes after the profile and every other table were replaced.
    QSqlQuery query(db.connection());
    REQUIRE(query.exec(QStringLiteral(
        "CREATE TRIGGER full_disk BEFORE INSERT ON achievement BEGIN SELECT RAISE(ABORT, 'disk full'); END")));

    lq::Profile restored = sampleProfile();
    restored.name = QStringLiteral("Ana");
    lq::LearnerState other = sampleLearner();
    other.done.clear();
    other.freezes = 2;
    QString error;
    CHECK_FALSE(lq::restoreLearner(db.connection(), restored, other, &error));
    CHECK(error.contains(QStringLiteral("disk full")));
    CHECK(lq::loadProfile(db.connection()) == sampleProfile());
    const auto loaded = lq::loadLearner(db.connection(), samplePlan());
    REQUIRE(loaded);
    CHECK(*loaded == before);
}

int main(int argc, char *argv[])
{
    // Qt SQL loads its SQLite driver as a plugin, which needs an application object.
    QCoreApplication app(argc, argv);
    return Catch::Session().run(argc, argv);
}
