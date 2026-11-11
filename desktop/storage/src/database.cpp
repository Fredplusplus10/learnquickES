#include "lq/database.h"

#include <QAtomicInt>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <vector>

namespace lq {

namespace {

// Entry i upgrades the schema from version i to i + 1, inside one transaction.
// Never edit a migration that has shipped: add a new one.
const std::vector<std::vector<const char *>> &migrations()
{
    static const std::vector<std::vector<const char *>> list{
        {
            // 1: the profile collected on the welcome screen. There is only ever one row.
            R"sql(
            CREATE TABLE profile (
                id          INTEGER PRIMARY KEY CHECK (id = 1),
                name        TEXT    NOT NULL,
                reason      TEXT    NOT NULL,
                goals       TEXT    NOT NULL DEFAULT '',
                level       TEXT    NOT NULL,
                plan_days   INTEGER NOT NULL CHECK (plan_days IN (30, 60, 90)),
                plan_start  TEXT    NOT NULL,
                study_days  TEXT    NOT NULL DEFAULT '1,2,3,4,5',
                created_at  TEXT    NOT NULL
            ))sql",
        },
        {
            // 2: learning progress. Dates are ISO calendar days ("2026-11-02"); the plan lives in the profile.
            R"sql(
            CREATE TABLE plan_day (
                n        INTEGER PRIMARY KEY,
                done_on  TEXT    NOT NULL
            ))sql",
            R"sql(
            CREATE TABLE task_done (
                n        INTEGER NOT NULL,
                task     TEXT    NOT NULL,
                done_on  TEXT    NOT NULL,
                PRIMARY KEY (n, task)
            ))sql",
            R"sql(
            CREATE TABLE unit_result (
                unit      INTEGER PRIMARY KEY CHECK (unit BETWEEN 0 AND 24),
                first_on  TEXT    NOT NULL,
                score     REAL    NOT NULL,
                best      REAL    NOT NULL,
                attempts  INTEGER NOT NULL
            ))sql",
            R"sql(
            CREATE TABLE checkpoint_result (
                phase     INTEGER PRIMARY KEY CHECK (phase BETWEEN 1 AND 5),
                first_on  TEXT    NOT NULL,
                score     REAL    NOT NULL,
                best      REAL    NOT NULL,
                attempts  INTEGER NOT NULL
            ))sql",
            R"sql(
            CREATE TABLE activity (
                day             TEXT    PRIMARY KEY,
                xp              INTEGER NOT NULL DEFAULT 0,
                seconds         INTEGER NOT NULL DEFAULT 0,
                input_minutes   INTEGER NOT NULL DEFAULT 0,
                island_minutes  INTEGER NOT NULL DEFAULT 0,
                speak           INTEGER NOT NULL DEFAULT 0
            ))sql",
            R"sql(
            CREATE TABLE answer_tally (
                day      TEXT    NOT NULL,
                skill    TEXT    NOT NULL,
                correct  INTEGER NOT NULL,
                total    INTEGER NOT NULL,
                PRIMARY KEY (day, skill)
            ))sql",
            R"sql(
            CREATE TABLE card (
                id           TEXT    PRIMARY KEY,
                seq          INTEGER NOT NULL UNIQUE,
                unit         INTEGER NOT NULL,
                box          INTEGER NOT NULL CHECK (box BETWEEN 1 AND 7),
                due          TEXT    NOT NULL,
                reps         INTEGER NOT NULL,
                lapses       INTEGER NOT NULL,
                added        TEXT    NOT NULL,
                last_review  TEXT
            ))sql",
            R"sql(
            CREATE TABLE streak (
                id       INTEGER PRIMARY KEY CHECK (id = 1),
                freezes  INTEGER NOT NULL,
                longest  INTEGER NOT NULL
            ))sql",
            R"sql(
            CREATE TABLE frozen_day (
                day  TEXT PRIMARY KEY
            ))sql",
        },
        {
            // 3: the coach. Every few sessions the learner says what they enjoyed, and practice leans towards it.
            // offered and liked are comma-separated exercise kinds ("listen,order"); feel is '' when not asked.
            R"sql(
            CREATE TABLE reflection (
                seq       INTEGER PRIMARY KEY,
                day       TEXT    NOT NULL,
                activity  TEXT    NOT NULL,
                offered   TEXT    NOT NULL DEFAULT '',
                liked     TEXT    NOT NULL DEFAULT '',
                feel      TEXT    NOT NULL DEFAULT '' CHECK (feel IN ('', 'easy', 'right', 'hard')),
                note      TEXT    NOT NULL DEFAULT ''
            ))sql",
            R"sql(
            CREATE TABLE session_count (
                id            INTEGER PRIMARY KEY CHECK (id = 1),
                finished      INTEGER NOT NULL,
                reflected_at  INTEGER NOT NULL
            ))sql",
        },
        {
            // 4: the emoji the learner picked for their streak; '' shows the app's own flame.
            "ALTER TABLE profile ADD COLUMN streak_emoji TEXT NOT NULL DEFAULT ''",
        },
        {
            // 5: Settings (day or night, the learner's voice, its paces), one row per setting.
            R"sql(
            CREATE TABLE setting (
                key    TEXT PRIMARY KEY,
                value  TEXT NOT NULL
            ))sql",
        },
        {
            // 6: immersion (listening and island minutes, conversations, devices in Spanish) and achievements;
            // the web app's holiday and day-off choices, which arrive with its backups.
            "ALTER TABLE activity ADD COLUMN talk_minutes INTEGER NOT NULL DEFAULT 0",
            "ALTER TABLE profile ADD COLUMN skip_holidays INTEGER NOT NULL DEFAULT 1",
            "ALTER TABLE profile ADD COLUMN days_off TEXT NOT NULL DEFAULT ''",
            R"sql(
            CREATE TABLE input_log (
                seq      INTEGER PRIMARY KEY,
                day      TEXT    NOT NULL,
                minutes  INTEGER NOT NULL CHECK (minutes > 0),
                kind     TEXT    NOT NULL,
                source   TEXT    NOT NULL DEFAULT ''
            ))sql",
            R"sql(
            CREATE TABLE exchange_log (
                seq      INTEGER PRIMARY KEY,
                day      TEXT    NOT NULL,
                minutes  INTEGER NOT NULL CHECK (minutes > 0),
                partner  TEXT    NOT NULL DEFAULT '',
                note     TEXT    NOT NULL DEFAULT ''
            ))sql",
            R"sql(
            CREATE TABLE mission (
                id   TEXT PRIMARY KEY,
                day  TEXT NOT NULL
            ))sql",
            R"sql(
            CREATE TABLE achievement (
                id   TEXT PRIMARY KEY,
                day  TEXT NOT NULL
            ))sql",
        },
        {
            // 7 (0.4.5): every voice now speaks at the same pace and the narrator changed, so the paces and
            // voice saved before mean something else; the learner starts again from the recommended ones.
            "DELETE FROM setting WHERE key IN ('voice', 'pace', 'slow_pace')",
        },
        {
            // 8: A1, A2 and B1 readiness certificates (lq/certificates.h). One per learner, level and course
            // revision; an award is written once and never changed or removed, so it keeps the name it was
            // issued to. A backup restore leaves this table alone.
            R"sql(
            CREATE TABLE certificate_award (
                certificate_id        TEXT PRIMARY KEY,
                profile_id            TEXT NOT NULL,
                level                 TEXT NOT NULL CHECK (level IN ('A1', 'A2', 'B1')),
                username_at_issue     TEXT NOT NULL CHECK (length(username_at_issue) > 0),
                curriculum_revision   TEXT NOT NULL,
                assessment_reference  TEXT NOT NULL CHECK (length(assessment_reference) > 0),
                issued_on             TEXT NOT NULL,
                UNIQUE (profile_id, level, curriculum_revision)
            ))sql",
            R"sql(
            CREATE TRIGGER certificate_award_no_update BEFORE UPDATE ON certificate_award
            BEGIN
                SELECT RAISE(ABORT, 'an issued certificate is never changed');
            END)sql",
            R"sql(
            CREATE TRIGGER certificate_award_no_delete BEFORE DELETE ON certificate_award
            BEGIN
                SELECT RAISE(ABORT, 'an issued certificate is never removed');
            END)sql",
        },
        {
            // 9: listening read from a streaming service's viewing history (lq/viewing.h). It counts on the day it
            // was watched but earned no XP, so taking it back must not take XP away.
            "ALTER TABLE input_log ADD COLUMN imported INTEGER NOT NULL DEFAULT 0",
        },
        {
            // 10: every attempt at a level assessment (lq/assessment.h), passed or not, with what each part
            // asked and got right. A certificate rests on one of these rows, so they are never changed or
            // removed, and a backup restore leaves them alone, like the awards.
            R"sql(
            CREATE TABLE assessment_result (
                reference            TEXT PRIMARY KEY,
                profile_id           TEXT    NOT NULL,
                level                TEXT    NOT NULL CHECK (level IN ('A1', 'A2', 'B1')),
                curriculum_revision  TEXT    NOT NULL,
                taken_on             TEXT    NOT NULL,
                reading_right        INTEGER NOT NULL CHECK (reading_right BETWEEN 0 AND reading_total),
                reading_total        INTEGER NOT NULL CHECK (reading_total > 0),
                listening_right      INTEGER NOT NULL CHECK (listening_right BETWEEN 0 AND listening_total),
                listening_total      INTEGER NOT NULL CHECK (listening_total > 0),
                language_right       INTEGER NOT NULL CHECK (language_right BETWEEN 0 AND language_total),
                language_total       INTEGER NOT NULL CHECK (language_total > 0)
            ))sql",
            "CREATE INDEX assessment_result_by_level ON assessment_result (profile_id, level, curriculum_revision)",
            R"sql(
            CREATE TRIGGER assessment_result_no_update BEFORE UPDATE ON assessment_result
            BEGIN
                SELECT RAISE(ABORT, 'an assessment result is never changed');
            END)sql",
            R"sql(
            CREATE TRIGGER assessment_result_no_delete BEFORE DELETE ON assessment_result
            BEGIN
                SELECT RAISE(ABORT, 'an assessment result is never removed');
            END)sql",
        },
    };
    return list;
}

QAtomicInt connectionCounter;

} // namespace

Database::Database()
    : m_connectionName(QStringLiteral("learnquick-%1").arg(connectionCounter.fetchAndAddRelaxed(1)))
{
}

Database::~Database()
{
    close();
}

int Database::latestSchemaVersion()
{
    return static_cast<int>(migrations().size());
}

bool Database::open(const QString &filePath)
{
    close();
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    db.setDatabaseName(filePath);
    if (!db.open()) {
        m_error = db.lastError().text();
        return false;
    }
    QSqlQuery(db).exec(QStringLiteral("PRAGMA foreign_keys = ON"));
    return migrate(filePath);
}

bool Database::backUpBeforeMigrating(const QString &filePath, int version)
{
    // One copy per schema version, kept: a second run (after a failed migration, say) finds it already there.
    const QString copy = QStringLiteral("%1.v%2.bak").arg(filePath).arg(version);
    if (QFileInfo(copy).isFile()) {
        m_migrationBackup = copy;
        return true;
    }
    // VACUUM INTO writes a consistent copy from the open connection, whatever the journal holds.
    QSqlQuery query(connection());
    if (!query.prepare(QStringLiteral("VACUUM INTO ?"))) {
        m_error = query.lastError().text();
        return false;
    }
    query.addBindValue(copy);
    if (!query.exec()) {
        m_error = QStringLiteral("Your data was not updated, because a copy could not be saved first (%1): %2")
                      .arg(copy, query.lastError().text());
        return false;
    }
    m_migrationBackup = copy;
    return true;
}

void Database::close()
{
    if (!QSqlDatabase::contains(m_connectionName))
        return;
    QSqlDatabase::database(m_connectionName, false).close();
    QSqlDatabase::removeDatabase(m_connectionName);
}

QSqlDatabase Database::connection() const
{
    return QSqlDatabase::database(m_connectionName, false);
}

int Database::schemaVersion() const
{
    QSqlQuery query(connection());
    return query.exec(QStringLiteral("PRAGMA user_version")) && query.next() ? query.value(0).toInt() : -1;
}

bool Database::migrate(const QString &filePath)
{
    QSqlDatabase db = connection();
    const int current = schemaVersion();
    m_migrationBackup.clear();
    if (current < 0 || current > latestSchemaVersion()) {
        m_error = QStringLiteral("Unsupported database schema version %1").arg(current);
        return false;
    }
    // A new file (schema 0) has nothing to lose; one with data is copied before anything changes.
    if (current > 0 && current < latestSchemaVersion() && !backUpBeforeMigrating(filePath, current))
        return false;
    // Upgrade all missing schema versions in one transaction. If any version fails, the
    // whole upgrade is rolled back instead of leaving an intermediate partially upgraded DB.
    if (current == latestSchemaVersion())
        return true;
    if (!db.transaction()) {
        m_error = db.lastError().text();
        return false;
    }
    for (int version = current; version < latestSchemaVersion(); ++version) {
        QSqlQuery query(db);
        for (const char *statement : migrations()[static_cast<std::size_t>(version)]) {
            if (!query.exec(QString::fromUtf8(statement))) {
                m_error = query.lastError().text();
                db.rollback();
                return false;
            }
        }
        if (!query.exec(QStringLiteral("PRAGMA user_version = %1").arg(version + 1))) {
            m_error = query.lastError().text();
            db.rollback();
            return false;
        }
    }
    if (!db.commit()) {
        m_error = db.lastError().text();
        db.rollback();
        return false;
    }
    return true;
}

} // namespace lq
