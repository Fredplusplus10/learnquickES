#pragma once

#include "lq/assessment.h"
#include "lq/coach.h"
#include "lq/course.h"
#include "lq/database.h"
#include "lq/progress.h"
#include "lq/viewing.h"
#include "profilecontroller.h"

#include <QDate>
#include <QDateTime>
#include <QObject>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

#include <functional>
#include <optional>

// The learner's progress and the course, shared by every screen. Each change goes through apply(): the
// engine updates the state, the review task is re-synced, the state is saved, and events become toasts.
class LearnerController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by the application")
    Q_PROPERTY(bool ready READ ready NOTIFY changed FINAL)
    Q_PROPERTY(bool demo READ demo CONSTANT FINAL)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged FINAL)
    Q_PROPERTY(int dueCount READ dueCount NOTIFY changed FINAL)
    Q_PROPERTY(int deckSize READ deckSize NOTIFY changed FINAL)
    Q_PROPERTY(QVariantList courseMap READ courseMap NOTIFY changed FINAL)
    Q_PROPERTY(QVariantList coachTips READ coachTips NOTIFY changed FINAL)
    Q_PROPERTY(bool reflectionDue READ reflectionDue NOTIFY changed FINAL)
    Q_PROPERTY(int reflectEvery READ reflectEvery CONSTANT FINAL)
    Q_PROPERTY(QString tasteText READ tasteText NOTIFY changed FINAL)
    Q_PROPERTY(QVariantList exerciseKinds READ exerciseKinds CONSTANT FINAL)

public:
    // demo: the sample learner, kept in memory only. coursePath: bundledCoursePath() in the app.
    LearnerController(ProfileController *profile, QDate today, bool demo, const QString &coursePath,
                      QObject *parent = nullptr);

    static QString bundledCoursePath();
    // What to tell the learner when the database refuses a save: the cause in plain words, and what to do.
    static QString saveProblem(const QString &databaseError);

    // Where progress is saved. Not called in demo mode or when the database could not be opened.
    void attach(lq::Database *database);

    bool ready() const { return m_state.has_value(); }
    bool demo() const { return m_demo; }
    QString error() const { return m_error; }
    int dueCount() const;
    int deckSize() const;
    QVariantList courseMap() const;

    QDate today() const { return m_today; }
    lq::Date day() const;
    const lq::Course *course() const { return m_course ? &*m_course : nullptr; }
    // The level assessments (assessments.json beside the course), or nullptr when the file is missing or damaged;
    // then certificates cannot be earned, and everything else works as before.
    const lq::Assessments *assessments() const { return m_assessments ? &*m_assessments : nullptr; }
    const lq::LearnerState &state() const { return *m_state; }
    const ProfileController *profile() const { return m_profile; }

    // Changes the state through the engine, then syncs review, saves and announces what happened.
    void apply(const std::function<lq::Events(lq::LearnerState &)> &change);
    // Ticks a trainer task if today's plan asks for it. Returns whether it is done now.
    bool completeTask(lq::Task task);

    // The coach's suggestions as {id, title, detail, route}, most useful first. `after`: the activity just
    // finished ("lesson" or a trainer id), which the coach does not send the learner straight back to.
    QVariantList coach(const QString &after) const;
    QVariantList coachTips() const { return coach({}); }
    bool reflectionDue() const;
    int reflectEvery() const { return lq::kReflectEvery; }
    // What the learner's taste changes, in one sentence; "" before their first reflection.
    QString tasteText() const;
    // Every kind of exercise as {id, label}, for telling the coach what you enjoy outside a session.
    QVariantList exerciseKinds() const;
    static QVariantList kindList(const QStringList &kinds);
    // A lesson or practice round finished. Every few, the learner is asked what they enjoyed.
    void countSession();

    // After a session: today's next task if one is left, otherwise the coach's first suggestion; {} if none.
    Q_INVOKABLE QVariantMap nextStep(const QString &after) const;
    // Saves a reflection and returns what it changes. feel: "easy", "right", "hard", or "" when not asked.
    Q_INVOKABLE QString reflect(const QString &activity, const QStringList &offered, const QStringList &liked,
                                const QString &feel, const QString &note);
    // "Not now": asked again after a few more sessions.
    Q_INVOKABLE void postponeReflection();

    // Lesson content for the lesson player.
    Q_INVOKABLE QVariantMap unit(int index) const;
    // Saves a finished lesson; returns {newCards, firstTime}.
    Q_INVOKABLE QVariantMap finishLesson(int unit, double score, int phrasesSaid);
    // Title, description and reference panels for a trainer ("verbs", "past", …, "review").
    Q_INVOKABLE QVariantMap trainer(const QString &id) const;
    Q_INVOKABLE QVariantMap checkpoint(int phase) const;
    Q_INVOKABLE QVariantMap mockExam() const;
    Q_INVOKABLE QVariantList writingChecks(const QString &text) const;
    // Scores the B1 mock and saves it as the phase 5 result. scores: reading, listening, grammar as
    // [correct, total], writing (points out of 7), speaking (list of 0–3 ratings).
    Q_INVOKABLE QVariantMap finishMock(const QVariantMap &scores);
    // The screen that runs a task of today's plan ("lesson/3", "practice/verbs", "checkpoint/2", "mock").
    Q_INVOKABLE QString routeForTask(const QString &taskId) const;

    // The screens to browse (src/learnerreports.cpp). Progress: the goal, pace and target date, the phases with
    // their plan days, weekly Spanish minutes and deck size, the summary, Spanish minutes for the last 14
    // days, first-try accuracy by skill, B1 readiness and every achievement, earned or not.
    Q_INVOKABLE QVariantMap progressReport() const;
    // Immersion: today's minutes against the plan's target, listening hours and the next milestone, the
    // newest log entries and exchanges, the device missions, resources, series tips and exchange phrases.
    Q_INVOKABLE QVariantMap immersion() const;
    // What the coach rests on: first-try answers by skill over the coach's window (two weeks), each marked
    // "enough" once it has the answers the coach needs to judge it; due cards, the plan pace, and the kinds of
    // exercise the reflections say the learner enjoys.
    Q_INVOKABLE QVariantMap coachReport() const;
    // Every course word with its lesson, phase and review state: "new", "learning", "remembered" or "due",
    // and a search key: Spanish and English lowercased, without accents or punctuation.
    Q_INVOKABLE QVariantList vocabulary() const;
    // The same key for what the learner types in the search box.
    Q_INVOKABLE QString searchKey(const QString &text) const;
    // kind: one of immersion().kinds. Minutes earn their number in XP.
    Q_INVOKABLE void logInput(int minutes, const QString &kind, const QString &source);
    // index: the entry's "index" in immersion().log. Takes back its minutes and XP.
    Q_INVOKABLE void removeInput(int index);
    Q_INVOKABLE void logExchange(int minutes, const QString &partner, const QString &note);
    Q_INVOKABLE void setMission(const QString &id, bool done);

    // A streaming service's viewing history (src/learnerviewing.cpp, lq/viewing.h). readViewing opens the file and
    // returns {service, hasMinutes, rows, skipped, first, last, planStart, profiles, shows: [{show, titles, minutes,
    // sinceStart, first, last, before}]}, or {error}. importViewing adds the chosen shows to the immersion log,
    // from the plan's start unless beforeStart, and returns {added, minutes, already}.
    Q_INVOKABLE QVariantMap readViewing(const QUrl &file);
    // The newest Netflix history in the Downloads folder from the last two weeks, {url, name, saved}, while it
    // holds days after the last viewing imported; {} otherwise. The file is read here and goes nowhere.
    Q_INVOKABLE QVariantMap downloadedViewing() const;
    QVariantMap downloadedViewingIn(const QString &folder, const QDateTime &now) const;
    Q_INVOKABLE QVariantMap importViewing(const QStringList &shows, const QString &profile, int minutesPerItem,
                                          bool beforeStart);

    // The backup file (src/learnerbackup.cpp), in the web app's format so either app opens it. Both return ""
    // when they worked, otherwise what went wrong, in words for the learner.
    Q_INVOKABLE QUrl backupSuggestion() const;  // Documents/learnquick-backup-<today>.json
    Q_INVOKABLE QString exportBackup(const QUrl &file) const;
    // Replaces the profile and all progress with the file's, then reloads. Refused for the sample learner.
    Q_INVOKABLE QString importBackup(const QUrl &file);

    // A word as the screens show it: {id, es, en, example, gender, article, word}.
    static QVariantMap wordMap(const lq::Vocab &v);

signals:
    void changed();
    void errorChanged();
    // A moment worth a toast: a streak extended, a plan day or a phase completed.
    void celebrated(const QString &title, const QString &detail, const QString &icon);

private:
    void load();
    bool save();
    void announce(const lq::Events &events);
    void setError(const QString &error);
    QVariantMap tipMap(const lq::CoachTip &tip) const;

    ProfileController *m_profile;
    QDate m_today;
    bool m_demo;
    std::optional<lq::Course> m_course;
    std::optional<lq::Assessments> m_assessments;
    QString m_courseError;
    lq::Database *m_database = nullptr;
    std::optional<lq::LearnerState> m_state;
    std::optional<lq::ViewingFile> m_viewing;  // the history read last, until it is imported
    QString m_error;
    QString m_saveProblem;  // why the last save failed; empty while saving works
};
