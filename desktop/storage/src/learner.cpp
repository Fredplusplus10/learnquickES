#include "lq/learner.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>

#include <tuple>
#include <utility>

namespace lq {

namespace {

QString iso(Date date)
{
    return QString::fromStdString(toIso(date));
}

// Thrown inside loadLearner when a stored date does not parse: the file was edited or damaged.
struct BadRow {
    QString what;
};

Date dateOf(const QVariant &value)
{
    const auto date = parseIso(value.toString().toStdString());
    if (!date)
        throw BadRow{QStringLiteral("invalid date \"%1\"").arg(value.toString())};
    return *date;
}

bool run(QSqlQuery &query, const QString &sql)
{
    return query.prepare(sql) && query.exec();
}

std::vector<std::string> kindsOf(const QVariant &value)
{
    std::vector<std::string> kinds;
    for (const QString &kind : value.toString().split(u',', Qt::SkipEmptyParts))
        kinds.push_back(kind.toStdString());
    return kinds;
}

// "" for none, never a null QString: Qt binds that as NULL, which the NOT NULL columns refuse.
QString joined(const std::vector<std::string> &kinds)
{
    std::string text;
    for (const std::string &kind : kinds)
        text += (text.empty() ? "" : ",") + kind;
    return QString::fromStdString(text);
}

} // namespace

std::optional<LearnerState> loadLearner(const QSqlDatabase &db, const Plan &plan, QString *error)
{
    LearnerState state;
    state.plan = plan;
    QSqlQuery q(db);
    const auto fail = [&](const QString &message) -> std::optional<LearnerState> {
        if (error)
            *error = message;
        return std::nullopt;
    };
    try {
        if (!run(q, QStringLiteral("SELECT n, done_on FROM plan_day ORDER BY n")))
            return fail(q.lastError().text());
        while (q.next())
            state.done.push_back({q.value(0).toInt(), dateOf(q.value(1))});

        if (!run(q, QStringLiteral("SELECT n, task, done_on FROM task_done")))
            return fail(q.lastError().text());
        while (q.next()) {
            if (const auto task = taskFromId(q.value(1).toString().toStdString()))
                state.tasks[q.value(0).toInt()][*task] = dateOf(q.value(2));
        }

        for (const auto &[table, key, target] : {std::tuple{"unit_result", "unit", &state.units},
                                                  std::tuple{"checkpoint_result", "phase", &state.checkpoints}}) {
            if (!run(q, QStringLiteral("SELECT %1, first_on, score, best, attempts FROM %2").arg(QLatin1StringView(key), QLatin1StringView(table))))
                return fail(q.lastError().text());
            while (q.next()) {
                (*target)[q.value(0).toInt()] = Score{dateOf(q.value(1)), q.value(2).toDouble(), q.value(3).toDouble(),
                                                      q.value(4).toInt()};
            }
        }

        if (!run(q, QStringLiteral(
                        "SELECT day, xp, seconds, input_minutes, island_minutes, speak, talk_minutes FROM activity")))
            return fail(q.lastError().text());
        while (q.next()) {
            Activity &a = state.activity[dateOf(q.value(0))];
            a.xp = q.value(1).toInt();
            a.seconds = q.value(2).toInt();
            a.inputMinutes = q.value(3).toInt();
            a.islandMinutes = q.value(4).toInt();
            a.speak = q.value(5).toInt();
            a.talkMinutes = q.value(6).toInt();
        }
        if (!run(q, QStringLiteral("SELECT day, skill, correct, total FROM answer_tally")))
            return fail(q.lastError().text());
        while (q.next()) {
            state.activity[dateOf(q.value(0))].answers[q.value(1).toString().toStdString()] =
                Tally{q.value(2).toInt(), q.value(3).toInt()};
        }

        if (!run(q, QStringLiteral("SELECT id, seq, unit, box, due, reps, lapses, added, last_review FROM card ORDER BY seq")))
            return fail(q.lastError().text());
        while (q.next()) {
            Card card;
            card.seq = q.value(1).toInt();
            card.unit = q.value(2).toInt();
            card.box = q.value(3).toInt();
            card.due = dateOf(q.value(4));
            card.reps = q.value(5).toInt();
            card.lapses = q.value(6).toInt();
            card.added = dateOf(q.value(7));
            if (!q.value(8).isNull())
                card.last = dateOf(q.value(8));
            state.cards.emplace(q.value(0).toString().toStdString(), card);
        }

        if (!run(q, QStringLiteral("SELECT freezes, longest FROM streak WHERE id = 1")))
            return fail(q.lastError().text());
        if (q.next()) {
            state.freezes = q.value(0).toInt();
            state.longest = q.value(1).toInt();
        }
        if (!run(q, QStringLiteral("SELECT day FROM frozen_day")))
            return fail(q.lastError().text());
        while (q.next())
            state.frozen.insert(dateOf(q.value(0)));

        if (!run(q, QStringLiteral("SELECT day, activity, offered, liked, feel, note FROM reflection ORDER BY seq")))
            return fail(q.lastError().text());
        while (q.next()) {
            state.reflections.push_back(Reflection{
                .date = dateOf(q.value(0)),
                .activity = q.value(1).toString().toStdString(),
                .offered = kindsOf(q.value(2)),
                .liked = kindsOf(q.value(3)),
                .feel = feelFromId(q.value(4).toString().toStdString()),
                .note = q.value(5).toString().toStdString(),
            });
        }
        if (!run(q, QStringLiteral("SELECT finished, reflected_at FROM session_count WHERE id = 1")))
            return fail(q.lastError().text());
        if (q.next()) {
            state.sessions = q.value(0).toInt();
            state.reflectedAt = q.value(1).toInt();
        }

        if (!run(q, QStringLiteral("SELECT day, minutes, kind, source, imported FROM input_log ORDER BY seq")))
            return fail(q.lastError().text());
        while (q.next()) {
            state.input.push_back({dateOf(q.value(0)), q.value(1).toInt(), q.value(2).toString().toStdString(),
                                   q.value(3).toString().toStdString(), q.value(4).toBool()});
        }
        if (!run(q, QStringLiteral("SELECT day, minutes, partner, note FROM exchange_log ORDER BY seq")))
            return fail(q.lastError().text());
        while (q.next()) {
            state.exchanges.push_back({dateOf(q.value(0)), q.value(1).toInt(), q.value(2).toString().toStdString(),
                                       q.value(3).toString().toStdString()});
        }
        for (const auto &[table, target] : {std::pair{"mission", &state.missions},
                                            std::pair{"achievement", &state.achievements}}) {
            if (!run(q, QStringLiteral("SELECT id, day FROM %1").arg(QLatin1StringView(table))))
                return fail(q.lastError().text());
            while (q.next())
                target->emplace(q.value(0).toString().toStdString(), dateOf(q.value(1)));
        }
    } catch (const BadRow &bad) {
        return fail(QStringLiteral("Your progress file has a damaged row: %1.").arg(bad.what));
    }
    return state;
}

namespace {

// One transaction for the whole snapshot, with the profile too when one is given.
bool writeLearner(const QSqlDatabase &constDb, const LearnerState &state, const Profile *profile, QString *error)
{
    QSqlDatabase db = constDb;
    if (!db.transaction()) {
        if (error)
            *error = db.lastError().text();
        return false;
    }
    QSqlQuery q(db);
    const auto fail = [&]() {
        if (error)
            *error = q.lastError().text();
        db.rollback();
        return false;
    };
    for (const char *table : {"plan_day", "task_done", "unit_result", "checkpoint_result", "activity", "answer_tally", "card",
                              "streak", "frozen_day", "reflection", "session_count", "input_log", "exchange_log",
                              "mission", "achievement"}) {
        if (!q.exec(QStringLiteral("DELETE FROM %1").arg(QLatin1StringView(table))))
            return fail();
    }
    if (profile && !saveProfile(db, *profile, error)) {
        db.rollback();
        return false;
    }

    q.prepare(QStringLiteral("INSERT INTO plan_day (n, done_on) VALUES (?, ?)"));
    for (const DoneDay &day : state.done) {
        q.addBindValue(day.n);
        q.addBindValue(iso(day.date));
        if (!q.exec())
            return fail();
    }
    q.prepare(QStringLiteral("INSERT INTO task_done (n, task, done_on) VALUES (?, ?, ?)"));
    for (const auto &[n, tasks] : state.tasks) {
        for (const auto &[task, date] : tasks) {
            const std::string_view id = taskId(task);
            q.addBindValue(n);
            q.addBindValue(QString::fromLatin1(id.data(), static_cast<qsizetype>(id.size())));
            q.addBindValue(iso(date));
            if (!q.exec())
                return fail();
        }
    }
    for (const auto &[table, key, scores] : {std::tuple{"unit_result", "unit", &state.units},
                                             std::tuple{"checkpoint_result", "phase", &state.checkpoints}}) {
        q.prepare(QStringLiteral("INSERT INTO %1 (%2, first_on, score, best, attempts) VALUES (?, ?, ?, ?, ?)").arg(QLatin1StringView(table), QLatin1StringView(key)));
        for (const auto &[index, score] : *scores) {
            q.addBindValue(index);
            q.addBindValue(iso(score.date));
            q.addBindValue(score.score);
            q.addBindValue(score.best);
            q.addBindValue(score.attempts);
            if (!q.exec())
                return fail();
        }
    }
    q.prepare(QStringLiteral("INSERT INTO activity (day, xp, seconds, input_minutes, island_minutes, speak, talk_minutes) "
                             "VALUES (?, ?, ?, ?, ?, ?, ?)"));
    for (const auto &[day, a] : state.activity) {
        q.addBindValue(iso(day));
        q.addBindValue(a.xp);
        q.addBindValue(a.seconds);
        q.addBindValue(a.inputMinutes);
        q.addBindValue(a.islandMinutes);
        q.addBindValue(a.speak);
        q.addBindValue(a.talkMinutes);
        if (!q.exec())
            return fail();
    }
    q.prepare(QStringLiteral("INSERT INTO answer_tally (day, skill, correct, total) VALUES (?, ?, ?, ?)"));
    for (const auto &[day, a] : state.activity) {
        for (const auto &[skill, tally] : a.answers) {
            q.addBindValue(iso(day));
            q.addBindValue(QString::fromStdString(skill));
            q.addBindValue(tally.correct);
            q.addBindValue(tally.total);
            if (!q.exec())
                return fail();
        }
    }
    q.prepare(QStringLiteral("INSERT INTO card (id, seq, unit, box, due, reps, lapses, added, last_review) "
                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    for (const auto &[id, card] : state.cards) {
        q.addBindValue(QString::fromStdString(id));
        q.addBindValue(card.seq);
        q.addBindValue(card.unit);
        q.addBindValue(card.box);
        q.addBindValue(iso(card.due));
        q.addBindValue(card.reps);
        q.addBindValue(card.lapses);
        q.addBindValue(iso(card.added));
        q.addBindValue(card.last ? QVariant(iso(*card.last)) : QVariant(QMetaType::fromType<QString>()));
        if (!q.exec())
            return fail();
    }
    q.prepare(QStringLiteral("INSERT INTO streak (id, freezes, longest) VALUES (1, ?, ?)"));
    q.addBindValue(state.freezes);
    q.addBindValue(state.longest);
    if (!q.exec())
        return fail();
    q.prepare(QStringLiteral("INSERT INTO frozen_day (day) VALUES (?)"));
    for (const Date day : state.frozen) {
        q.addBindValue(iso(day));
        if (!q.exec())
            return fail();
    }
    q.prepare(QStringLiteral("INSERT INTO reflection (seq, day, activity, offered, liked, feel, note) VALUES (?, ?, ?, ?, ?, ?, ?)"));
    for (std::size_t i = 0; i < state.reflections.size(); ++i) {
        const Reflection &r = state.reflections[i];
        const std::string feel(r.feel ? feelId(*r.feel) : std::string_view());
        q.addBindValue(static_cast<int>(i));
        q.addBindValue(iso(r.date));
        q.addBindValue(QString::fromStdString(r.activity));
        q.addBindValue(joined(r.offered));
        q.addBindValue(joined(r.liked));
        q.addBindValue(QString::fromStdString(feel));
        q.addBindValue(QString::fromStdString(r.note));
        if (!q.exec())
            return fail();
    }
    q.prepare(QStringLiteral("INSERT INTO session_count (id, finished, reflected_at) VALUES (1, ?, ?)"));
    q.addBindValue(state.sessions);
    q.addBindValue(state.reflectedAt);
    if (!q.exec())
        return fail();
    q.prepare(QStringLiteral("INSERT INTO input_log (seq, day, minutes, kind, source, imported) VALUES (?, ?, ?, ?, ?, ?)"));
    for (std::size_t i = 0; i < state.input.size(); ++i) {
        const InputEntry &entry = state.input[i];
        q.addBindValue(static_cast<int>(i));
        q.addBindValue(iso(entry.date));
        q.addBindValue(entry.minutes);
        q.addBindValue(QString::fromStdString(entry.kind));
        q.addBindValue(QString::fromStdString(entry.source));
        q.addBindValue(entry.imported ? 1 : 0);
        if (!q.exec())
            return fail();
    }
    q.prepare(QStringLiteral("INSERT INTO exchange_log (seq, day, minutes, partner, note) VALUES (?, ?, ?, ?, ?)"));
    for (std::size_t i = 0; i < state.exchanges.size(); ++i) {
        const Exchange &exchange = state.exchanges[i];
        q.addBindValue(static_cast<int>(i));
        q.addBindValue(iso(exchange.date));
        q.addBindValue(exchange.minutes);
        q.addBindValue(QString::fromStdString(exchange.partner));
        q.addBindValue(QString::fromStdString(exchange.note));
        if (!q.exec())
            return fail();
    }
    for (const auto &[table, dates] : {std::pair{"mission", &state.missions},
                                       std::pair{"achievement", &state.achievements}}) {
        q.prepare(QStringLiteral("INSERT INTO %1 (id, day) VALUES (?, ?)").arg(QLatin1StringView(table)));
        for (const auto &[id, date] : *dates) {
            q.addBindValue(QString::fromStdString(id));
            q.addBindValue(iso(date));
            if (!q.exec())
                return fail();
        }
    }
    if (!db.commit()) {
        if (error)
            *error = db.lastError().text();
        db.rollback();
        return false;
    }
    return true;
}

} // namespace

bool saveLearner(const QSqlDatabase &db, const LearnerState &state, QString *error)
{
    return writeLearner(db, state, nullptr, error);
}

bool restoreLearner(const QSqlDatabase &db, const Profile &profile, const LearnerState &state, QString *error)
{
    return writeLearner(db, state, &profile, error);
}

} // namespace lq
