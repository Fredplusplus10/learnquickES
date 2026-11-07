#include "learnercontroller.h"
#include "lq/database.h"
#include "preferencescontroller.h"
#include "profilecontroller.h"
#include "qtdate.h"
#include "sessionmodel.h"
#include "todaymodel.h"

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include "lq/learner.h"
#include "lq/viewing.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QVariantMap>

#include <algorithm>
#include <chrono>
#include <cmath>

namespace {

const QString kCourse = QStringLiteral(LQ_CONTENT_DIR "/course.json");
const QString kReason = QStringLiteral("I want to talk with my coworkers in Mexico without switching to English.");

lq::Profile freshProfile(QDate start, int planDays)
{
    lq::Profile profile;
    profile.name = QStringLiteral("Cory");
    profile.reason = kReason;
    profile.level = QStringLiteral("A2");
    profile.planDays = planDays;
    profile.planStart = start;
    return profile;
}

QString calendarState(const TodayModel &model, int index)
{
    return model.calendar().at(index).toMap().value(QStringLiteral("state")).toString();
}

QVariantMap task(const TodayModel &model, const QString &id)
{
    for (const QVariant &t : model.tasks()) {
        if (t.toMap().value(QStringLiteral("id")).toString() == id)
            return t.toMap();
    }
    return {};
}

// Plays a whole session the way the screen would: right answers on choices and matches, "I don't know"
// on typed answers and tiles, "good" on review cards, "said it" on speaking, and the second-try round.
void playThrough(SessionModel &session)
{
    for (int guard = 0; session.active() && !session.finished() && guard < 500; ++guard) {
        if (session.intermission()) {
            session.startSecondTry();
            continue;
        }
        const QVariantMap item = session.item();
        const QString type = item.value(QStringLiteral("type")).toString();
        if (type == u"match") {
            for (int i = 0; i < item.value(QStringLiteral("pairs")).toList().size(); ++i) {
                session.tapMatch(QStringLiteral("left"), i);
                session.tapMatch(QStringLiteral("right"), i);
            }
        } else if (type == u"gender") {
            session.chooseArticle(item.value(QStringLiteral("article")).toString());
        } else if (type == u"card") {
            session.reveal();
            session.grade(2);
            continue;
        } else if (type == u"speak") {
            session.selfRate(true);
            continue;
        } else if (type == u"describe") {
            session.reveal();
            session.selfRate(true);
            continue;
        } else if (type == u"choice") {
            session.select(item.value(QStringLiteral("answer")).toInt());
            session.check();
        } else {
            session.skip();
        }
        REQUIRE(session.checked());
        session.next();
    }
    REQUIRE(session.finished());
}

// A learner who finished the welcome flow on `start`, with progress saved in `path`.
struct Learner {
    lq::Database db;
    ProfileController profile;
    LearnerController learner;
    TodayModel today;
    SessionModel session;

    Learner(const QString &path, QDate start, QDate day, int planDays = 30)
        : profile(day)
        , learner(&profile, day, false, kCourse)
        , today(&learner)
        , session(&learner, 1234)
    {
        REQUIRE(db.open(path));
        profile.attach(&db);
        learner.attach(&db);
        if (!profile.exists())
            REQUIRE(profile.save(QStringLiteral("Cory"), kReason, {}, QStringLiteral("A2+"), planDays));
        REQUIRE(profile.profile()->planStart == start);
        REQUIRE(learner.ready());
    }
};

} // namespace

TEST_CASE("welcome answers are validated, saved, and still there after a restart", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    const QDate start(2026, 11, 2);
    {
        lq::Database db;
        REQUIRE(db.open(path));
        ProfileController profile(start);
        profile.attach(&db);
        CHECK_FALSE(profile.exists());

        // One word is not a reason.
        CHECK_FALSE(profile.save(QStringLiteral("Cory"), QStringLiteral("work"), {}, QStringLiteral("A2"), 60));
        CHECK(profile.error().contains(QStringLiteral("why")));
        CHECK_FALSE(profile.exists());

        REQUIRE(profile.save(QStringLiteral("  Cory "), kReason, {QStringLiteral("work")}, QStringLiteral("A2"), 60));
        CHECK(profile.error().isEmpty());
        CHECK(profile.name() == QStringLiteral("Cory"));
    }

    // Three days later the app opens straight on Today, with the plan start unchanged.
    lq::Database db;
    REQUIRE(db.open(path));
    ProfileController profile(start.addDays(3));
    profile.attach(&db);
    REQUIRE(profile.exists());
    CHECK(profile.reason() == kReason);
    CHECK(profile.profile()->planStart == start);
    CHECK(profile.profile()->goals == QStringList{QStringLiteral("work")});
}

TEST_CASE("without a working database the welcome flow refuses to save instead of losing the answers", "[app]")
{
    ProfileController profile(QDate(2026, 11, 2));
    profile.setError(QStringLiteral("Your learner data could not be opened."));
    CHECK_FALSE(profile.save(QStringLiteral("Cory"), kReason, {}, QStringLiteral("A2"), 60));
    CHECK(profile.error() == QStringLiteral("Your learner data could not be opened."));
    CHECK_FALSE(profile.exists());
}

TEST_CASE("Today appears as soon as the welcome flow is saved", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    lq::Database db;
    REQUIRE(db.open(dir.filePath(QStringLiteral("learnquick.sqlite3"))));
    const QDate today(2026, 11, 2);
    ProfileController profile(today);
    profile.attach(&db);
    LearnerController learner(&profile, today, false, kCourse);
    learner.attach(&db);
    TodayModel model(&learner);
    CHECK(model.dayNumber() == 0);

    REQUIRE(profile.save(QStringLiteral("Cory"), kReason, {}, QStringLiteral("A2+"), 30));
    CHECK(model.learnerName() == QStringLiteral("Cory"));
    CHECK(model.reason() == kReason);
    CHECK(model.totalDays() == 30);
}

TEST_CASE("a new learner's Today starts at day 1 of their plan", "[app]")
{
    const QDate today(2026, 11, 2);
    ProfileController profile(today);
    profile.preview(freshProfile(today, 30));
    LearnerController learner(&profile, today, false, kCourse);
    const TodayModel model(&learner);

    REQUIRE(learner.error().isEmpty());
    CHECK(model.dateText() == QStringLiteral("Monday, November 2"));
    CHECK(model.dayNumber() == 1);
    CHECK(model.totalDays() == 30);
    CHECK(model.phaseTitle() == QStringLiteral("Foundations 80/20"));
    CHECK(model.dayTitle() == QStringLiteral("Ser & Estar: Who and How You Are"));
    CHECK(model.dayTitleEs() == QStringLiteral("Ser y estar"));
    // The 30-day plan adds a focus trainer to every lesson day.
    REQUIRE(model.tasks().size() == 4);
    CHECK(model.tasks().at(1).toMap().value(QStringLiteral("name")).toString().startsWith(QStringLiteral("Lesson 1: ")));
    // Each task opens the screen that runs it.
    CHECK(task(model, QStringLiteral("lesson")).value(QStringLiteral("route")) == QStringLiteral("lesson/0"));
    CHECK(task(model, QStringLiteral("verbs")).value(QStringLiteral("route")) == QStringLiteral("practice/verbs"));
    // An empty deck has nothing to review, so that task is already done.
    CHECK(task(model, QStringLiteral("review")).value(QStringLiteral("done")).toBool());
    CHECK(model.minutes() == 5 + 20 + 5 + 6);
    CHECK(model.streak() == 0);
    CHECK(model.doneCount() == 0);
    CHECK(model.paceText() == QStringLiteral("On track"));
    CHECK(model.inputMinutes() == 90);

    // Five weeks ending this week: today is the first day of the last row.
    REQUIRE(model.calendar().size() == 35);
    CHECK(calendarState(model, 28) == QStringLiteral("today"));
    CHECK(calendarState(model, 27) == QStringLiteral("before"));  // the plan had not started
    CHECK(calendarState(model, 29) == QStringLiteral("future"));
}

TEST_CASE("the sample learner shows the same numbers as the web app", "[app]")
{
    const QDate today(2026, 11, 2);
    ProfileController profile(today);
    profile.preview(ProfileController::demoProfile(today));
    LearnerController learner(&profile, today, true, kCourse);
    const TodayModel model(&learner);

    CHECK(model.dayNumber() == 16);
    CHECK(model.doneCount() == 15);
    CHECK(model.streak() == 17);
    CHECK(model.freezes() == 2);
    CHECK(model.paceText() == QStringLiteral("1 day behind"));
    CHECK(model.targetText() == QStringLiteral("Jan 7, 2027"));
    // Every due card, like the web app's words.due; one review session takes the first 20 (checked in parity).
    CHECK(learner.dueCount() == 37);
}

TEST_CASE("a missing course file is reported, not hidden", "[app]")
{
    ProfileController profile(QDate(2026, 11, 2));
    profile.preview(freshProfile(QDate(2026, 11, 2), 30));
    const LearnerController learner(&profile, QDate(2026, 11, 2), false, QStringLiteral("does-not-exist.json"));
    CHECK_FALSE(learner.error().isEmpty());
    CHECK_FALSE(learner.ready());
}

TEST_CASE("a finished lesson and its review cards are still there after a restart", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    const QDate start(2026, 11, 2);
    std::size_t words = 0;
    {
        Learner cory(path, start, start);
        cory.session.startLesson(0);
        REQUIRE(cory.session.active());
        playThrough(cory.session);
        const QVariantMap result = cory.session.result();
        CHECK(result.value(QStringLiteral("answered")).toInt() > 0);
        CHECK(result.value(QStringLiteral("xp")).toInt() > 0);

        const QVariantMap saved = cory.learner.finishLesson(0, result.value(QStringLiteral("score")).toDouble(), 2);
        words = cory.learner.course()->units[0].vocab.size();
        CHECK(saved.value(QStringLiteral("newCards")).toInt() == static_cast<int>(words));
        CHECK(saved.value(QStringLiteral("firstTime")).toBool());
        CHECK(task(cory.today, QStringLiteral("lesson")).value(QStringLiteral("done")).toBool());
        CHECK(cory.learner.courseMap().at(0).toMap().value(QStringLiteral("done")).toInt() == 1);
    }

    // Next morning: the lesson is done, its words are due for the first review.
    Learner cory(path, start, start.addDays(1));
    const lq::LearnerState &state = cory.learner.state();
    CHECK(state.units.contains(0));
    CHECK(state.cards.size() == words);
    CHECK(state.activity.at(lq::Date(std::chrono::year(2026) / 11 / 2)).speak == 2);
    CHECK(cory.learner.dueCount() == static_cast<int>(words));
    // Review was ticked yesterday with an empty deck; now that cards are due it is open again.
    CHECK_FALSE(task(cory.today, QStringLiteral("review")).value(QStringLiteral("done")).toBool());
}

TEST_CASE("reviewing every due card saves the grades and ticks the review task", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    const QDate start(2026, 11, 2);
    {
        Learner cory(path, start, start);
        cory.learner.finishLesson(0, 1.0, 0);
    }
    {
        Learner cory(path, start, start.addDays(1));
        REQUIRE(cory.learner.dueCount() > 0);
        cory.session.startTrainer(QStringLiteral("review"));
        playThrough(cory.session);
        CHECK(cory.session.result().value(QStringLiteral("marked")).toBool());
        CHECK(cory.learner.dueCount() == 0);
        CHECK(task(cory.today, QStringLiteral("review")).value(QStringLiteral("done")).toBool());
    }
    Learner cory(path, start, start.addDays(1));
    for (const auto &[id, card] : cory.learner.state().cards) {
        CAPTURE(id);
        CHECK(card.box == 2);
        CHECK(card.reps == 1);
    }
}

TEST_CASE("an empty review finishes at once and says when the next card is due", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QDate start(2026, 11, 2);
    Learner cory(dir.filePath(QStringLiteral("learnquick.sqlite3")), start, start);
    cory.learner.finishLesson(0, 1.0, 0);
    cory.session.startTrainer(QStringLiteral("review"));
    CHECK(cory.session.finished());
    CHECK(cory.session.result().value(QStringLiteral("empty")).toBool());
    CHECK(cory.learner.trainer(QStringLiteral("review")).value(QStringLiteral("nextDue")) == QStringLiteral("Nov 3"));
}

TEST_CASE("a trainer round ticks its task in today's plan", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QDate start(2026, 11, 2);
    Learner cory(dir.filePath(QStringLiteral("learnquick.sqlite3")), start, start);
    CHECK_FALSE(task(cory.today, QStringLiteral("verbs")).value(QStringLiteral("done")).toBool());
    cory.session.startTrainer(QStringLiteral("verbs"));
    CHECK(cory.session.count() == 10);
    CHECK(cory.session.item().value(QStringLiteral("type")) == QStringLiteral("conj"));
    playThrough(cory.session);
    CHECK(cory.session.result().value(QStringLiteral("marked")).toBool());
    CHECK(task(cory.today, QStringLiteral("verbs")).value(QStringLiteral("done")).toBool());
    // Every answer was "I don't know": each came back once, and the misses are listed for review.
    CHECK(cory.session.result().value(QStringLiteral("percent")).toInt() == 0);
    CHECK(cory.session.result().value(QStringLiteral("missed")).toList().size() == 6);

    // A trainer that is not on today's plan still counts as practice, but ticks nothing.
    cory.session.startTrainer(QStringLiteral("describe"));
    playThrough(cory.session);
    CHECK_FALSE(cory.session.result().value(QStringLiteral("marked")).toBool());
    CHECK(cory.session.result().value(QStringLiteral("selfRated")).toInt() == 5);
}

TEST_CASE("misses wait for a second-try round at the end, worth more than before but less than a first try", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QDate start(2026, 11, 2);
    Learner cory(dir.filePath(QStringLiteral("learnquick.sqlite3")), start, start);
    SessionModel &session = cory.session;
    session.startTrainer(QStringLiteral("verbs"));
    REQUIRE(session.count() == 10);

    // "I don't know" on every item: the counter keeps its length and nothing comes back in between.
    QStringList expected;
    for (int i = 0; i < 10; ++i) {
        REQUIRE_FALSE(session.intermission());
        CHECK_FALSE(session.secondTry());
        CHECK(session.position() == i);
        CHECK(session.count() == 10);
        CHECK_FALSE(session.item().value(QStringLiteral("retry")).toBool());
        session.skip();
        REQUIRE(session.checked());
        expected << session.feedback().value(QStringLiteral("expected")).toString();
        session.next();
    }

    // The round waits behind its own screen, where answering does nothing.
    REQUIRE(session.intermission());
    CHECK_FALSE(session.finished());
    CHECK(session.item().isEmpty());
    CHECK(session.secondTryCount() == 10);
    CHECK(session.secondTryXp() == lq::kXpRetry);
    CHECK(lq::kXpRetry > 5);
    CHECK(lq::kXpRetry < lq::kXpCorrect);
    session.skip();
    session.next();
    CHECK(session.intermission());

    session.startSecondTry();
    CHECK_FALSE(session.intermission());
    CHECK(session.secondTry());
    CHECK(session.position() == 0);
    CHECK(session.count() == 10);
    CHECK(session.item().value(QStringLiteral("retry")).toBool());

    // Right on the second try: kXpRetry, and it is counted as fixed.
    const int xp = cory.learner.state().activity.at(cory.learner.day()).xp;
    session.setInput(expected.front());
    session.check();
    REQUIRE(session.feedback().value(QStringLiteral("ok")).toBool());
    CHECK(cory.learner.state().activity.at(cory.learner.day()).xp == xp + lq::kXpRetry);
    session.next();
    CHECK(session.position() == 1);
    playThrough(session);
    const QVariantMap result = session.result();
    CHECK(result.value(QStringLiteral("percent")).toInt() == 0);  // the score counts first tries only
    CHECK(result.value(QStringLiteral("secondTries")).toInt() == 10);
    CHECK(result.value(QStringLiteral("fixed")).toInt() == 1);

    // The round can be skipped from its screen: the session ends with what was done.
    session.startTrainer(QStringLiteral("verbs"));
    for (int i = 0; i < 10; ++i) {
        session.skip();
        session.next();
    }
    REQUIRE(session.intermission());
    session.skipSecondTry();
    CHECK(session.finished());
    CHECK(session.result().value(QStringLiteral("secondTries")).toInt() == 0);
}

TEST_CASE("a checkpoint is saved with its score and the lessons behind the misses", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QDate start(2026, 11, 2);
    Learner cory(dir.filePath(QStringLiteral("learnquick.sqlite3")), start, start);
    cory.session.startCheckpoint(1);
    CHECK(cory.session.count() == 15);
    playThrough(cory.session);
    CHECK(cory.session.count() == 15);  // no second tries
    const QVariantMap result = cory.session.result();
    CHECK(result.contains(QStringLiteral("passed")));
    CHECK(cory.learner.state().checkpoints.contains(1));
    for (const QVariant &unit : result.value(QStringLiteral("weakUnits")).toList())
        CHECK(unit.toMap().value(QStringLiteral("index")).toInt() < 5);
    CHECK(cory.learner.checkpoint(1).value(QStringLiteral("best")).toInt() >= 0);
}

TEST_CASE("the B1 mock is scored and saved as the final result", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QDate start(2026, 11, 2);
    Learner cory(dir.filePath(QStringLiteral("learnquick.sqlite3")), start, start);
    const QVariantMap result = cory.learner.finishMock({
        {QStringLiteral("reading"), QVariantList{5, 5}},
        {QStringLiteral("listening"), QVariantList{4, 5}},
        {QStringLiteral("grammar"), QVariantList{8, 10}},
        {QStringLiteral("writing"), 6},
        {QStringLiteral("speaking"), QVariantList{3, 2, 2, 3}},
    });
    CHECK(result.value(QStringLiteral("pass")).toBool());
    CHECK(result.value(QStringLiteral("reading")).toInt() == 87);  // (5 + 8) / 15
    CHECK(cory.learner.state().checkpoints.contains(lq::kPhases));
    CHECK(cory.learner.mockExam().value(QStringLiteral("best")).toInt() == result.value(QStringLiteral("overall")).toInt());

    const QVariantList checks = cory.learner.writingChecks(QStringLiteral("Ayer fui al mercado, pero estaba cerrado. Luego compré pan y mi amigo llegó tarde."));
    REQUIRE(checks.size() == 4);
    CHECK(checks.at(1).toMap().value(QStringLiteral("ok")).toBool());  // past tenses found
}

TEST_CASE("a level assessment runs part by part, marks nothing on screen and records nothing as practice", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QDate start(2026, 11, 2);
    Learner cory(dir.filePath(QStringLiteral("learnquick.sqlite3")), start, start);
    REQUIRE(cory.learner.assessments() != nullptr);
    CHECK(cory.session.startAssessment(QStringLiteral("C1"), QStringLiteral("seed")).isEmpty());

    const QVariantMap draw = cory.session.startAssessment(QStringLiteral("A2"), QStringLiteral("profile/A2/2026-11-02"));
    CHECK(draw.value(QStringLiteral("level")) == QStringLiteral("A2"));
    CHECK_FALSE(draw.value(QStringLiteral("readingText")).toString().isEmpty());
    CHECK_FALSE(draw.value(QStringLiteral("listeningText")).toString().isEmpty());
    CHECK_FALSE(draw.value(QStringLiteral("listeningIntro")).toString().isEmpty());
    CHECK(draw.value(QStringLiteral("language")).toInt() == lq::kAssessmentLanguageItems);
    // The same seed brings back the same texts.
    CHECK(cory.session.startAssessment(QStringLiteral("A2"), QStringLiteral("profile/A2/2026-11-02")) == draw);

    const lq::LearnerState before = cory.learner.state();
    for (const QString part : {QStringLiteral("reading"), QStringLiteral("listening"), QStringLiteral("language")}) {
        INFO(part.toStdString());
        cory.session.startAssessmentPart(part);
        REQUIRE(cory.session.active());
        CHECK(cory.session.kind() == QStringLiteral("assessment"));
        const int asked = cory.session.count();
        for (int guard = 0; !cory.session.finished() && guard < 50; ++guard) {
            const QVariantMap item = cory.session.item();
            CHECK(item.value(QStringLiteral("type")) == QStringLiteral("choice"));
            CHECK_FALSE(item.contains(QStringLiteral("answer")));  // the right option never reaches the screen
            cory.session.select(0);
            cory.session.check();
            // Straight on to the next question: nothing says whether it was right.
            CHECK(cory.session.feedback().isEmpty());
            if (!cory.session.finished())
                CHECK_FALSE(cory.session.checked());
        }
        REQUIRE(cory.session.finished());
        const QVariantMap result = cory.session.result();
        CHECK(result.value(QStringLiteral("part")) == part);
        CHECK(result.value(QStringLiteral("answered")).toInt() == asked);
        CHECK_FALSE(result.contains(QStringLiteral("missed")));
        CHECK(result.value(QStringLiteral("secondTries")).toInt() == 0);
        CHECK(result.value(QStringLiteral("xp")).toInt() == 0);
    }
    CHECK(cory.learner.state() == before);  // no XP, no answers by skill, no sessions counted
    // Without a draw, a part does not start.
    SessionModel fresh(&cory.learner, 7);
    fresh.startAssessmentPart(QStringLiteral("reading"));
    CHECK_FALSE(fresh.active());
}

TEST_CASE("every third session asks what the learner enjoyed, and the answer is kept", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    const QDate start(2026, 11, 2);
    {
        Learner cory(path, start, start);
        for (const QString trainer : {QStringLiteral("verbs"), QStringLiteral("describe")}) {
            cory.session.startTrainer(trainer);
            playThrough(cory.session);
            CHECK_FALSE(cory.session.result().value(QStringLiteral("reflect")).toBool());
        }
        cory.session.startTrainer(QStringLiteral("speak"));
        playThrough(cory.session);
        const QVariantMap result = cory.session.result();
        REQUIRE(result.value(QStringLiteral("reflect")).toBool());
        CHECK(result.value(QStringLiteral("activity")) == QStringLiteral("speak"));
        const QVariantList kinds = result.value(QStringLiteral("kinds")).toList();
        REQUIRE(kinds.size() == 1);
        CHECK(kinds.at(0).toMap().value(QStringLiteral("id")) == QStringLiteral("speak"));
        CHECK(kinds.at(0).toMap().value(QStringLiteral("label")) == QStringLiteral("Speaking out loud"));

        const QString reply = cory.learner.reflect(QStringLiteral("speak"), {QStringLiteral("speak")}, {QStringLiteral("speak")},
                                                   QStringLiteral("easy"), QStringLiteral("  Me gustó   hablar. "));
        CHECK(reply.startsWith(QStringLiteral("Mixed practice now leans towards speaking out loud.")));
        CHECK_FALSE(cory.learner.reflectionDue());
        CHECK(cory.learner.tasteText() == reply);
    }

    // Still there after a restart, with the note tidied.
    Learner cory(path, start, start);
    const lq::LearnerState &state = cory.learner.state();
    CHECK(state.sessions == 3);
    REQUIRE(state.reflections.size() == 1);
    CHECK(state.reflections[0].note == "Me gustó hablar.");
    CHECK(state.reflections[0].feel == lq::Feel::Easy);

    // With a taste, mixed practice is still a round of ten.
    cory.session.startTrainer(QStringLiteral("mixed"));
    CHECK(cory.session.count() == 10);
    playThrough(cory.session);

    // "Not now" waits for three more sessions.
    for (int i = 0; i < lq::kReflectEvery; ++i)
        cory.learner.countSession();
    REQUIRE(cory.learner.reflectionDue());
    cory.learner.postponeReflection();
    CHECK_FALSE(cory.learner.reflectionDue());
}

TEST_CASE("a lesson's reflection covers its exercises and its speaking step", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QDate start(2026, 11, 2);
    Learner cory(dir.filePath(QStringLiteral("learnquick.sqlite3")), start, start);
    cory.session.startLesson(0);
    playThrough(cory.session);
    QStringList kinds;
    for (const QVariant &kind : cory.session.result().value(QStringLiteral("kinds")).toList())
        kinds << kind.toMap().value(QStringLiteral("id")).toString();
    CHECK(kinds.contains(QStringLiteral("match")));
    CHECK(kinds.last() == QStringLiteral("speak"));
    CHECK(cory.session.result().value(QStringLiteral("activity")) == QStringLiteral("lesson"));
}

TEST_CASE("the coach points at the next task after a session, and its suggestions open screens", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QDate start(2026, 11, 2);
    Learner cory(dir.filePath(QStringLiteral("learnquick.sqlite3")), start, start);
    // Day 1: the lesson comes first.
    REQUIRE_FALSE(cory.today.coach().isEmpty());
    CHECK(cory.today.coach().at(0).toMap().value(QStringLiteral("route")) == QStringLiteral("lesson/0"));

    cory.session.startTrainer(QStringLiteral("verbs"));
    playThrough(cory.session);
    const QVariantMap next = cory.learner.nextStep(QStringLiteral("verbs"));
    CHECK(next.value(QStringLiteral("id")) == QStringLiteral("plan"));
    CHECK(next.value(QStringLiteral("title")).toString().startsWith(QStringLiteral("Next on today's plan: Lesson 1, ")));
    CHECK(next.value(QStringLiteral("route")) == QStringLiteral("lesson/0"));

    // The sample learner has 37 cards due: review comes first, and opens the review.
    const QDate today(2026, 11, 2);
    ProfileController profile(today);
    profile.preview(ProfileController::demoProfile(today));
    LearnerController learner(&profile, today, true, kCourse);
    const TodayModel model(&learner);
    REQUIRE_FALSE(model.coach().isEmpty());
    CHECK(model.coach().size() <= 3);
    CHECK(model.coach().at(0).toMap().value(QStringLiteral("id")) == QStringLiteral("review"));
    CHECK(model.coach().at(0).toMap().value(QStringLiteral("route")) == QStringLiteral("practice/review"));
    CHECK(learner.coachTips().size() >= model.coach().size());
    CHECK(learner.tasteText().startsWith(QStringLiteral("Mixed practice now leans towards listening")));
    CHECK(learner.exerciseKinds().size() == 11);
}

TEST_CASE("the sample learner can practise but never writes the database", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    const QDate today(2026, 11, 2);
    {
        lq::Database db;
        REQUIRE(db.open(path));
        ProfileController profile(today);
        profile.preview(ProfileController::demoProfile(today));
        LearnerController learner(&profile, today, true, kCourse);
        learner.attach(&db);
        const std::size_t before = learner.state().units.size();
        learner.finishLesson(20, 1.0, 0);
        CHECK(learner.state().units.size() == before + 1);
    }
    lq::Database db;
    REQUIRE(db.open(path));
    const auto stored = lq::loadLearner(db.connection(), lq::Plan{});
    REQUIRE(stored.has_value());
    CHECK(stored->units.empty());
    CHECK(stored->activity.empty());
}

TEST_CASE("settings are saved as they change, and the reset keeps day or night", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    {
        lq::Database db;
        REQUIRE(db.open(path));
        PreferencesController preferences(nullptr);
        preferences.attach(&db);
        CHECK(preferences.theme() == QStringLiteral("system"));
        CHECK(preferences.voice() == QStringLiteral("M2"));
        CHECK(preferences.recommendedVoice());
        CHECK(preferences.voices().size() == static_cast<qsizetype>(lq::voiceChoices().size()));
        // Settings marks one voice Recommended: the one a new learner starts with, which is on offer.
        CHECK(preferences.defaultVoice() == preferences.voice());
        const QVariantList offered = preferences.voices();
        CHECK(std::count_if(offered.cbegin(), offered.cend(), [&](const QVariant &v) {
                  return v.toMap().value(QStringLiteral("style")).toString() == preferences.defaultVoice();
              }) == 1);

        int changes = 0;
        QObject::connect(&preferences, &PreferencesController::changed, [&] { ++changes; });
        preferences.setTheme(QStringLiteral("dark"));
        preferences.setVoice(QStringLiteral("M5"));
        preferences.setPace(1.3);  // slow, 1.35, moves to stay slower than normal
        preferences.setSlowPace(1.4);  // still too close: no change
        preferences.setVoice(QStringLiteral("nobody"));  // not on offer: back to the recommended voice
        CHECK(changes == 4);
        CHECK(preferences.slowPace() == 1.45);
        CHECK(preferences.voice() == QStringLiteral("M2"));
        preferences.setVoice(QStringLiteral("M5"));
        CHECK_FALSE(preferences.recommendedVoice());
        CHECK(preferences.syllablesPerSecond(1.0) == lq::kReferenceSyllablesPerSecond);
        CHECK(preferences.error().isEmpty());
    }
    {
        lq::Database db;
        REQUIRE(db.open(path));
        PreferencesController preferences(nullptr);
        preferences.attach(&db);
        CHECK(preferences.theme() == QStringLiteral("dark"));
        CHECK(preferences.voice() == QStringLiteral("M5"));
        CHECK(preferences.pace() == 1.3);
        preferences.resetVoice();
        CHECK(preferences.recommendedVoice());
        CHECK(preferences.theme() == QStringLiteral("dark"));
    }
    lq::Database db;
    REQUIRE(db.open(path));
    CHECK(lq::loadSettings(db.connection()).voice == QStringLiteral("M2"));
}

TEST_CASE("a theme shown for a screenshot is never saved", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    lq::Database db;
    REQUIRE(db.open(dir.filePath(QStringLiteral("learnquick.sqlite3"))));
    PreferencesController preferences(nullptr);
    preferences.attach(&db);
    lq::Settings dark = preferences.settings();
    dark.theme = QStringLiteral("dark");
    preferences.preview(dark);
    preferences.setVoice(QStringLiteral("M1"));
    CHECK(preferences.theme() == QStringLiteral("dark"));
    CHECK(preferences.voice() == QStringLiteral("M1"));
    CHECK(lq::loadSettings(db.connection()) == lq::Settings{});
}

TEST_CASE("a backup from Settings brings everything back, and one from the web app restores its progress", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QDate start(2026, 11, 2);
    const QUrl file = QUrl::fromLocalFile(dir.filePath(QStringLiteral("backup.json")));
    Learner cory(dir.filePath(QStringLiteral("learnquick.sqlite3")), start, start);
    cory.learner.finishLesson(0, 0.9, 3);
    cory.learner.logInput(25, QStringLiteral("podcast"), QStringLiteral("No Hay Tos"));
    REQUIRE(cory.profile.setStreakEmoji(QStringLiteral("🌮")));
    const lq::LearnerState saved = cory.learner.state();
    CHECK(cory.learner.exportBackup(file).isEmpty());

    cory.learner.logInput(40, QStringLiteral("series"), {});
    REQUIRE(cory.profile.setStreakEmoji({}));
    CHECK(cory.learner.importBackup(file).isEmpty());
    CHECK(cory.learner.state() == saved);
    CHECK(cory.profile.streakEmoji() == QStringLiteral("🌮"));

    // Not a backup: refused with a reason, and nothing changes.
    const QUrl junk = QUrl::fromLocalFile(dir.filePath(QStringLiteral("notes.json")));
    {
        QFile out(junk.toLocalFile());
        REQUIRE(out.open(QIODevice::WriteOnly));
        out.write("{\"shopping\": [\"tortillas\"]}");
    }
    CHECK_FALSE(cory.learner.importBackup(junk).isEmpty());
    CHECK(cory.learner.state() == saved);

    // The 8 MB limit: a real backup padded to exactly the limit still restores; one byte more is refused.
    {
        QFile in(file.toLocalFile());
        REQUIRE(in.open(QIODevice::ReadOnly));
        QByteArray padded = in.readAll();
        constexpr qsizetype limit = 8 * 1024 * 1024;
        REQUIRE(padded.size() < limit);
        padded.append(QByteArray(limit - padded.size(), ' '));
        const QUrl big = QUrl::fromLocalFile(dir.filePath(QStringLiteral("big.json")));
        QFile out(big.toLocalFile());
        REQUIRE(out.open(QIODevice::WriteOnly));
        out.write(padded);
        out.close();
        CHECK(cory.learner.importBackup(big).isEmpty());
        CHECK(cory.learner.state() == saved);
        REQUIRE(out.open(QIODevice::Append));
        out.write(" ");
        out.close();
        CHECK(cory.learner.importBackup(big).contains(u"too large"));
    }

    // The web app's sample learner (tools/web_reference.js): its plan and progress, with Cory's own reason.
    QFile reference(QStringLiteral(LQ_TEST_DATA_DIR "/web_reference.json"));
    REQUIRE(reference.open(QIODevice::ReadOnly));
    const QJsonObject web = QJsonDocument::fromJson(reference.readAll()).object().value(QStringLiteral("backup")).toObject();
    const QUrl webFile = QUrl::fromLocalFile(dir.filePath(QStringLiteral("web.json")));
    {
        QFile out(webFile.toLocalFile());
        REQUIRE(out.open(QIODevice::WriteOnly));
        out.write(QJsonDocument(web).toJson());
    }
    CHECK(cory.learner.importBackup(webFile).isEmpty());
    const lq::Profile &profile = *cory.profile.profile();
    CHECK(profile.reason == kReason);
    CHECK(profile.level == QStringLiteral("A2"));
    CHECK(profile.planDays == 60);
    CHECK(profile.planStart.toString(Qt::ISODate)
          == web.value(QStringLiteral("state")).toObject().value(QStringLiteral("plan")).toObject().value(QStringLiteral("start")).toString());
    CHECK(cory.learner.state().cards.size() == 84);
    CHECK(cory.learner.state().exchanges.size() == 1);
}

TEST_CASE("days off and holidays set in the web app survive a restore and shape the plan", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QDate start(2026, 11, 2);
    Learner cory(dir.filePath(QStringLiteral("learnquick.sqlite3")), start, start);
    const QUrl file = QUrl::fromLocalFile(dir.filePath(QStringLiteral("backup.json")));
    REQUIRE(cory.learner.exportBackup(file).isEmpty());
    QFile in(file.toLocalFile());
    REQUIRE(in.open(QIODevice::ReadOnly));
    QJsonObject backup = QJsonDocument::fromJson(in.readAll()).object();
    in.close();
    QJsonObject state = backup.value(QStringLiteral("state")).toObject();
    QJsonObject plan = state.value(QStringLiteral("plan")).toObject();
    plan.insert(QStringLiteral("skipHolidays"), false);
    plan.insert(QStringLiteral("daysOff"), QJsonArray{QStringLiteral("2026-11-04")});
    state.insert(QStringLiteral("plan"), plan);
    backup.insert(QStringLiteral("state"), state);
    {
        QFile out(file.toLocalFile());
        REQUIRE(out.open(QIODevice::WriteOnly));
        out.write(QJsonDocument(backup).toJson());
    }

    CHECK(cory.learner.importBackup(file).isEmpty());
    CHECK_FALSE(cory.profile.profile()->skipHolidays);
    CHECK(cory.profile.profile()->daysOff == QList<QDate>{QDate(2026, 11, 4)});
    const lq::Plan &restored = cory.learner.state().plan;
    CHECK_FALSE(restored.skipHolidays);
    CHECK(restored.daysOff.contains(lq::Date(std::chrono::year(2026) / 11 / 4)));
}

TEST_CASE("the sample learner can be saved to a file but never replaced by one", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QDate today(2026, 11, 2);
    ProfileController profile(today);
    profile.preview(ProfileController::demoProfile(today));
    LearnerController learner(&profile, today, true, kCourse);
    const QUrl file = QUrl::fromLocalFile(dir.filePath(QStringLiteral("backup.json")));
    CHECK(learner.exportBackup(file).isEmpty());
    CHECK_FALSE(learner.importBackup(file).isEmpty());
    CHECK(learner.backupSuggestion().fileName() == QStringLiteral("learnquick-backup-2026-11-02.json"));
}

int main(int argc, char *argv[])
{
    // Qt SQL loads its SQLite driver as a plugin, which needs an application object.
    QCoreApplication app(argc, argv);
    return Catch::Session().run(argc, argv);
}

TEST_CASE("listening, devices and conversations are logged, kept after a restart, and earn achievements", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    const QDate start(2026, 11, 2);
    {
        Learner cory(path, start, start);
        QStringList toasts;
        QObject::connect(&cory.learner, &LearnerController::celebrated,
                         [&toasts](const QString &title, const QString &, const QString &) { toasts.append(title); });
        cory.learner.logInput(25, QStringLiteral("podcast"), QStringLiteral("No Hay Tos"));
        cory.learner.logInput(10, QStringLiteral("music"), {});
        cory.learner.logExchange(30, QStringLiteral("Mariana"), QStringLiteral("Comida"));
        cory.learner.setMission(QStringLiteral("phone"), true);
        CHECK(toasts.contains(QStringLiteral("Achievement: First Exchange")));

        QVariantMap immersion = cory.learner.immersion();
        CHECK(immersion.value(QStringLiteral("todayMinutes")).toInt() == 65);
        CHECK(immersion.value(QStringLiteral("missionsDone")).toInt() == 1);
        // Cory chose A2+ at the welcome screen: A2 series first, and the phase 4 checkpoint opens B1.
        CHECK(immersion.value(QStringLiteral("seriesLevel")).toString() == u"A2");
        CHECK(immersion.value(QStringLiteral("seriesWhy")).toString().contains(u"phase 4"));
        CHECK(immersion.value(QStringLiteral("series")).toList().size() == 5);
        const QVariantList log = immersion.value(QStringLiteral("log")).toList();
        REQUIRE(log.size() == 2);
        CHECK(log.at(0).toMap().value(QStringLiteral("kind")).toString() == u"Music");  // newest first
        cory.learner.removeInput(log.at(0).toMap().value(QStringLiteral("index")).toInt());
        CHECK(cory.learner.immersion().value(QStringLiteral("todayMinutes")).toInt() == 55);
    }

    Learner cory(path, start, start.addDays(1));
    const lq::LearnerState &state = cory.learner.state();
    REQUIRE(state.input.size() == 1);
    CHECK(state.input[0].source == "No Hay Tos");
    CHECK(state.exchanges.size() == 1);
    CHECK(state.missions.contains("phone"));
    CHECK(state.achievements.contains("exchange_1"));

    const QVariantMap report = cory.learner.progressReport();
    CHECK(report.value(QStringLiteral("earnedCount")).toInt() == 1);
    const QVariantList minutes = report.value(QStringLiteral("minutes")).toList();
    REQUIRE(minutes.size() == 14);
    CHECK(minutes.at(12).toMap().value(QStringLiteral("minutes")).toInt() == 55);  // yesterday
    CHECK(report.value(QStringLiteral("achievements")).toList().size() == 16);
}

TEST_CASE("a Netflix viewing history adds the shows watched in Spanish, kept after a restart, once", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    const QString csv = dir.filePath(QStringLiteral("NetflixViewingHistory.csv"));
    {
        QFile file(csv);
        REQUIRE(file.open(QIODevice::WriteOnly));
        file.write("Title,Date\n\"\xC3\x89lite: Temporada 1: Bienvenidos\",10/30/26\n"
                   "\"\xC3\x89lite: Temporada 1: Tarde de copas\",11/2/26\n"
                   "\"Friends: Season 1: The One Where Monica Gets a Roommate\",11/2/26\n");
    }
    const QDate start(2026, 11, 1);
    const QDate today(2026, 11, 3);
    {
        const Learner created(path, start, start);  // the plan starts the day the profile is made
    }
    {
        Learner cory(path, start, today);
        CHECK(cory.learner.importViewing({QStringLiteral("Élite")}, {}, 40, false).contains(QStringLiteral("error")));
        // Found in the Downloads folder while nothing in it was imported.
        const QVariantMap found = cory.learner.downloadedViewingIn(dir.path(), QDateTime::currentDateTime());
        CHECK(found.value(QStringLiteral("name")).toString() == u"NetflixViewingHistory.csv");
        CHECK(found.value(QStringLiteral("saved")).toString().startsWith(u"today at "));
        CHECK(cory.learner.downloadedViewingIn(dir.path(), QDateTime::currentDateTime().addDays(15)).isEmpty());
        // The personal-information download, unzipped with a folder of the same name inside, is found too.
        const QString nested = dir.filePath(QStringLiteral("downloads/netflix-report/netflix-report/CONTENT_INTERACTION"));
        REQUIRE(QDir().mkpath(nested));
        REQUIRE(QFile::copy(csv, nested + QStringLiteral("/ViewingActivity.csv")));
        CHECK(cory.learner.downloadedViewingIn(dir.filePath(QStringLiteral("downloads")), QDateTime::currentDateTime())
                  .value(QStringLiteral("name")).toString() == u"ViewingActivity.csv");

        const QVariantMap read = cory.learner.readViewing(QUrl::fromLocalFile(csv));
        REQUIRE_FALSE(read.contains(QStringLiteral("error")));
        CHECK(read.value(QStringLiteral("service")).toString() == u"Netflix");
        CHECK_FALSE(read.value(QStringLiteral("hasMinutes")).toBool());
        const QVariantList shows = read.value(QStringLiteral("shows")).toList();
        REQUIRE(shows.size() == 2);
        CHECK(shows.at(0).toMap().value(QStringLiteral("show")).toString() == u"Élite");  // made in Spanish first
        CHECK(shows.at(0).toMap().value(QStringLiteral("spanish")).toBool());
        CHECK_FALSE(shows.at(1).toMap().value(QStringLiteral("spanish")).toBool());
        CHECK(shows.at(0).toMap().value(QStringLiteral("titles")).toInt() == 2);
        CHECK(shows.at(0).toMap().value(QStringLiteral("sinceStart")).toInt() == 1);  // 10/30 is before the plan
        CHECK_FALSE(shows.at(0).toMap().value(QStringLiteral("before")).toBool());

        const int xpBefore = cory.learner.state().activity.contains(toLqDate(today))
                                 ? cory.learner.state().activity.at(toLqDate(today)).xp : 0;
        const QVariantMap done = cory.learner.importViewing({QStringLiteral("Élite")}, {}, 40, false);
        CHECK(done.value(QStringLiteral("added")).toInt() == 1);
        CHECK(done.value(QStringLiteral("minutes")).toInt() == 40);
        CHECK(cory.learner.immersion().value(QStringLiteral("listeningHours")).toDouble() > 0);
        const auto &activity = cory.learner.state().activity;
        CHECK(activity.at(toLqDate(QDate(2026, 11, 2))).xp == 0);
        CHECK((activity.contains(toLqDate(today)) ? activity.at(toLqDate(today)).xp : 0) == xpBefore);
        // Imported up to its last day, it is no longer offered.
        CHECK(cory.learner.downloadedViewingIn(dir.path(), QDateTime::currentDateTime()).isEmpty());
    }

    // After a restart: still there, marked as imported, and the same file adds nothing new.
    Learner cory(path, start, today);
    REQUIRE(cory.learner.state().input.size() == 1);
    CHECK(cory.learner.state().input[0].imported);
    const QVariantMap again = cory.learner.readViewing(QUrl::fromLocalFile(csv));
    CHECK(again.value(QStringLiteral("shows")).toList().at(0).toMap().value(QStringLiteral("before")).toBool());
    const QVariantMap twice = cory.learner.importViewing({QStringLiteral("Élite")}, {}, 40, true);
    CHECK(twice.value(QStringLiteral("added")).toInt() == 1);  // only 10/30, now that earlier days count
    CHECK(twice.value(QStringLiteral("already")).toInt() == 1);

    CHECK(cory.learner.readViewing(QUrl::fromLocalFile(dir.filePath(QStringLiteral("missing.csv"))))
              .contains(QStringLiteral("error")));

    // The size limit holds while reading: a real history padded to exactly the limit still reads, one byte more
    // is refused.
    QFile original(csv);
    REQUIRE(original.open(QIODevice::ReadOnly));
    QByteArray padded = original.readAll();
    original.close();
    const auto limit = static_cast<qsizetype>(lq::kMaxViewingBytes);
    REQUIRE(padded.size() < limit);
    padded.append(QByteArray(limit - padded.size(), '\n'));
    const QString big = dir.filePath(QStringLiteral("big.csv"));
    QFile bigFile(big);
    REQUIRE(bigFile.open(QIODevice::WriteOnly));
    REQUIRE(bigFile.write(padded) == limit);
    bigFile.close();
    const QVariantMap atLimit = cory.learner.readViewing(QUrl::fromLocalFile(big));
    CHECK_FALSE(atLimit.contains(QStringLiteral("error")));
    CHECK(atLimit.value(QStringLiteral("shows")).toList().size() == 2);
    REQUIRE(bigFile.open(QIODevice::Append));
    REQUIRE(bigFile.write("\n") == 1);
    bigFile.close();
    CHECK(cory.learner.readViewing(QUrl::fromLocalFile(big)).value(QStringLiteral("error")).toString()
          == u"This file is too large to be a viewing history.");
}

TEST_CASE("the vocabulary lists every course word with where it stands in review", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QDate start(2026, 11, 2);
    Learner cory(dir.filePath(QStringLiteral("learnquick.sqlite3")), start, start);
    cory.learner.finishLesson(0, 1.0, 0);
    const QVariantList words = cory.learner.vocabulary();
    CHECK(words.size() == 300);
    const QVariantMap first = words.at(0).toMap();
    CHECK(first.value(QStringLiteral("lesson")).toInt() == 1);
    CHECK(first.value(QStringLiteral("phase")).toInt() == 1);
    CHECK(first.value(QStringLiteral("state")).toString() == u"learning");  // due tomorrow
    CHECK(words.last().toMap().value(QStringLiteral("state")).toString() == u"new");
}

TEST_CASE("Progress shows the plan's goal, pace and phases, and weekly charts only from recorded days", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QDate start(2026, 11, 2);  // a Monday
    {
        Learner cory(dir.filePath(QStringLiteral("learnquick.sqlite3")), start, start, 60);
        const QVariantMap report = cory.learner.progressReport();

        // Day one: no pace to speak of yet, and nothing to chart.
        const QVariantMap pace = report.value(QStringLiteral("pace")).toMap();
        CHECK_FALSE(pace.value(QStringLiteral("enough")).toBool());
        CHECK(pace.value(QStringLiteral("text")).toString() == QStringLiteral("Not enough data"));
        const QVariantMap goal = report.value(QStringLiteral("goal")).toMap();
        CHECK(goal.value(QStringLiteral("days")).toInt() == 60);
        CHECK(goal.value(QStringLiteral("started")).toString() == QStringLiteral("Nov 2, 2026"));
        CHECK_FALSE(goal.value(QStringLiteral("target")).toString().isEmpty());
        CHECK(report.value(QStringLiteral("mostWeekMinutes")).toInt() == 0);
        CHECK(report.value(QStringLiteral("deckWords")).toInt() == 0);
        CHECK(report.value(QStringLiteral("spoken")).toInt() == 0);
        const QVariantList weeks = report.value(QStringLiteral("weeks")).toList();
        REQUIRE(weeks.size() == 1);  // only the week the learner started
        CHECK(weeks.at(0).toMap().value(QStringLiteral("current")).toBool());

        // The five phases share out the 60 plan days in order, the first one under way.
        const QVariantList roadmap = report.value(QStringLiteral("roadmap")).toList();
        REQUIRE(roadmap.size() == 5);
        CHECK(roadmap.at(0).toMap().value(QStringLiteral("state")).toString() == QStringLiteral("current"));
        CHECK(roadmap.at(0).toMap().value(QStringLiteral("days")).toString().startsWith(QStringLiteral("Day 1 – ")));
        CHECK(roadmap.at(4).toMap().value(QStringLiteral("days")).toString().endsWith(QStringLiteral(" – 60")));
        for (int i = 1; i < 5; ++i)
            CHECK(roadmap.at(i).toMap().value(QStringLiteral("state")).toString() == QStringLiteral("upcoming"));

        // A finished lesson puts its words in this week's deck and its time in this week's bar.
        cory.learner.finishLesson(0, 1.0, 3);
        const QVariantMap after = cory.learner.progressReport();
        const int words = after.value(QStringLiteral("deckWords")).toInt();
        CHECK(words > 0);
        CHECK(after.value(QStringLiteral("weeks")).toList().at(0).toMap().value(QStringLiteral("words")).toInt() == words);
        CHECK(after.value(QStringLiteral("spoken")).toInt() == 3);
    }
    {
        // Ten days later, a week and a half of study days has passed with one plan day done: now pace speaks.
        Learner cory(dir.filePath(QStringLiteral("learnquick.sqlite3")), start, QDate(2026, 11, 12), 60);
        const QVariantMap report = cory.learner.progressReport();
        const QVariantMap pace = report.value(QStringLiteral("pace")).toMap();
        CHECK(pace.value(QStringLiteral("enough")).toBool());
        CHECK(pace.value(QStringLiteral("diff")).toInt() < 0);
        CHECK(pace.value(QStringLiteral("text")).toString().endsWith(QStringLiteral(" behind")));
        const QVariantList weeks = report.value(QStringLiteral("weeks")).toList();
        REQUIRE(weeks.size() == 2);
        CHECK(weeks.at(0).toMap().value(QStringLiteral("label")).toString() == QStringLiteral("Nov 2"));
        CHECK(weeks.at(1).toMap().value(QStringLiteral("current")).toBool());
        // The deck never shrinks from one week to the next.
        CHECK(weeks.at(1).toMap().value(QStringLiteral("words")).toInt() >= weeks.at(0).toMap().value(QStringLiteral("words")).toInt());
    }
}

TEST_CASE("the sample learner's Progress matches its Today", "[app]")
{
    const QDate today(2026, 11, 2);
    ProfileController profile(today);
    profile.preview(ProfileController::demoProfile(today));
    LearnerController learner(&profile, today, true, kCourse);
    const QVariantMap report = learner.progressReport();
    const QVariantMap pace = report.value(QStringLiteral("pace")).toMap();
    CHECK(pace.value(QStringLiteral("text")).toString() == QStringLiteral("1 study day behind"));
    CHECK(report.value(QStringLiteral("goal")).toMap().value(QStringLiteral("target")).toString() == QStringLiteral("Jan 7, 2027"));
    const QVariantList roadmap = report.value(QStringLiteral("roadmap")).toList();
    REQUIRE(roadmap.size() == 5);
    CHECK(roadmap.at(0).toMap().value(QStringLiteral("state")).toString() == QStringLiteral("done"));
    CHECK(roadmap.at(1).toMap().value(QStringLiteral("state")).toString() == QStringLiteral("current"));
    CHECK(report.value(QStringLiteral("weeks")).toList().size() <= 12);
}

TEST_CASE("the coach's snapshot says when there is too little to judge, and matches its suggestions", "[app]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QDate start(2026, 11, 2);
    {
        // A new learner: no answers, nothing due, no pace and no taste yet, and nothing judged.
        Learner cory(dir.filePath(QStringLiteral("learnquick.sqlite3")), start, start, 60);
        const QVariantMap report = cory.learner.coachReport();
        CHECK(report.value(QStringLiteral("answers")).toInt() == 0);
        CHECK(report.value(QStringLiteral("due")).toInt() == 0);
        CHECK(report.value(QStringLiteral("pace")).toString() == QStringLiteral("Not enough data yet"));
        CHECK(report.value(QStringLiteral("favourites")).toList().isEmpty());
        CHECK(report.value(QStringLiteral("windowDays")).toInt() == lq::kSkillWindow);
        CHECK(report.value(QStringLiteral("minAnswers")).toInt() == lq::kSkillMinAnswers);
        const QVariantList skills = report.value(QStringLiteral("skills")).toList();
        REQUIRE_FALSE(skills.isEmpty());
        // Cognates and false friends are separate skills, so Practice shows each trainer its own figure.
        QStringList skillIds;
        for (const QVariant &skill : skills)
            skillIds << skill.toMap().value(QStringLiteral("id")).toString();
        CHECK(skillIds.contains(QStringLiteral("cognates")));
        CHECK(skillIds.contains(QStringLiteral("falsefriends")));
        for (const QVariant &skill : skills) {
            CHECK(skill.toMap().value(QStringLiteral("total")).toInt() == 0);
            CHECK(skill.toMap().value(QStringLiteral("accuracy")).toInt() == -1);
            CHECK_FALSE(skill.toMap().value(QStringLiteral("enough")).toBool());
        }
    }

    // The sample learner: every figure agrees with itself and with what the coach suggests.
    const QDate today(2026, 11, 2);
    ProfileController profile(today);
    profile.preview(ProfileController::demoProfile(today));
    LearnerController learner(&profile, today, true, kCourse);
    const QVariantMap report = learner.coachReport();
    CHECK(report.value(QStringLiteral("pace")).toString() == QStringLiteral("1 study day behind"));
    int answers = 0;
    bool weak = false;
    for (const QVariant &entry : report.value(QStringLiteral("skills")).toList()) {
        const QVariantMap skill = entry.toMap();
        const int correct = skill.value(QStringLiteral("correct")).toInt();
        const int total = skill.value(QStringLiteral("total")).toInt();
        answers += total;
        CHECK(correct <= total);
        CHECK(skill.value(QStringLiteral("enough")).toBool() == (total >= lq::kSkillMinAnswers));
        if (total > 0)
            CHECK(skill.value(QStringLiteral("accuracy")).toInt() == static_cast<int>(std::lround(100.0 * correct / total)));
        weak = weak || (skill.value(QStringLiteral("enough")).toBool() && skill.value(QStringLiteral("id")).toString() != QStringLiteral("review")
                        && skill.value(QStringLiteral("accuracy")).toInt() < report.value(QStringLiteral("weakBelow")).toInt());
    }
    CHECK(answers == report.value(QStringLiteral("answers")).toInt());
    CHECK(answers > 0);
    const int due = report.value(QStringLiteral("due")).toInt();
    bool reviewTip = false;
    bool skillTip = false;
    for (const QVariant &tip : learner.coachTips()) {
        const QString id = tip.toMap().value(QStringLiteral("id")).toString();
        reviewTip = reviewTip || id == QStringLiteral("review");
        skillTip = skillTip || id == QStringLiteral("skill");
        if (id == QStringLiteral("review"))
            CHECK(tip.toMap().value(QStringLiteral("detail")).toString().startsWith(QString::number(due) + QStringLiteral(" cards are due")));
    }
    CHECK(reviewTip == (due >= lq::kReviewPile));
    CHECK(skillTip == weak);
}
