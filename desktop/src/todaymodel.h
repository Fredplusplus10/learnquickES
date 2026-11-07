#pragma once

#include "learnercontroller.h"

#include <QDate>
#include <QObject>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

// Everything the Today screen shows, computed by the C++ engine for one learner and one date.
class TodayModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by the application")
    Q_PROPERTY(QString learnerName READ learnerName NOTIFY changed FINAL)
    Q_PROPERTY(QString reason READ reason NOTIFY changed FINAL)
    Q_PROPERTY(QString dateText READ dateText NOTIFY changed FINAL)
    Q_PROPERTY(int dayNumber READ dayNumber NOTIFY changed FINAL)
    Q_PROPERTY(int totalDays READ totalDays NOTIFY changed FINAL)
    Q_PROPERTY(bool finished READ finished NOTIFY changed FINAL)
    Q_PROPERTY(int phase READ phase NOTIFY changed FINAL)
    Q_PROPERTY(QString phaseTitle READ phaseTitle NOTIFY changed FINAL)
    Q_PROPERTY(QString dayKind READ dayKind NOTIFY changed FINAL)
    Q_PROPERTY(QString dayTitle READ dayTitle NOTIFY changed FINAL)
    Q_PROPERTY(QString dayTitleEs READ dayTitleEs NOTIFY changed FINAL)
    Q_PROPERTY(int minutes READ minutes NOTIFY changed FINAL)
    Q_PROPERTY(int inputMinutes READ inputMinutes NOTIFY changed FINAL)
    Q_PROPERTY(QVariantList tasks READ tasks NOTIFY changed FINAL)
    Q_PROPERTY(int streak READ streak NOTIFY changed FINAL)
    Q_PROPERTY(int longestStreak READ longestStreak NOTIFY changed FINAL)
    Q_PROPERTY(int freezes READ freezes NOTIFY changed FINAL)
    Q_PROPERTY(int maxFreezes READ maxFreezes CONSTANT FINAL)
    Q_PROPERTY(bool todayDone READ todayDone NOTIFY changed FINAL)
    Q_PROPERTY(QVariantList week READ week NOTIFY changed FINAL)
    Q_PROPERTY(int doneCount READ doneCount NOTIFY changed FINAL)
    Q_PROPERTY(int paceDiff READ paceDiff NOTIFY changed FINAL)
    Q_PROPERTY(QString paceText READ paceText NOTIFY changed FINAL)
    Q_PROPERTY(QString targetText READ targetText NOTIFY changed FINAL)
    Q_PROPERTY(QVariantList phases READ phases NOTIFY changed FINAL)
    Q_PROPERTY(QVariantList calendar READ calendar NOTIFY changed FINAL)
    Q_PROPERTY(QString calendarTitle READ calendarTitle NOTIFY changed FINAL)
    Q_PROPERTY(bool finishedToday READ finishedToday NOTIFY changed FINAL)
    Q_PROPERTY(QVariantList coach READ coach NOTIFY changed FINAL)

public:
    explicit TodayModel(LearnerController *learner, QObject *parent = nullptr);

    QString learnerName() const { return m_view.learnerName; }
    QString reason() const { return m_view.reason; }
    QString dateText() const { return m_view.dateText; }
    int dayNumber() const { return m_view.dayNumber; }
    int totalDays() const { return m_view.totalDays; }
    bool finished() const { return m_view.finished; }
    int phase() const { return m_view.phase; }
    QString phaseTitle() const { return m_view.phaseTitle; }
    QString dayKind() const { return m_view.dayKind; }
    QString dayTitle() const { return m_view.dayTitle; }
    QString dayTitleEs() const { return m_view.dayTitleEs; }
    int minutes() const { return m_view.minutes; }
    int inputMinutes() const { return m_view.inputMinutes; }
    QVariantList tasks() const { return m_view.tasks; }
    int streak() const { return m_view.streak; }
    int longestStreak() const { return m_view.longestStreak; }
    int freezes() const { return m_view.freezes; }
    int maxFreezes() const;
    bool todayDone() const { return m_view.todayDone; }
    QVariantList week() const { return m_view.week; }
    int doneCount() const { return m_view.doneCount; }
    int paceDiff() const { return m_view.paceDiff; }
    QString paceText() const { return m_view.paceText; }
    QString targetText() const { return m_view.targetText; }
    QVariantList phases() const { return m_view.phases; }
    QVariantList calendar() const { return m_view.calendar; }
    QString calendarTitle() const { return m_view.calendarTitle; }
    bool finishedToday() const { return m_view.finishedToday; }
    // The coach's first suggestions, {id, title, detail, route}.
    QVariantList coach() const { return m_view.coach; }

signals:
    void changed();

private:
    void refresh();

    struct View {
        QString learnerName, reason, dateText;
        int dayNumber = 0, totalDays = 0;
        bool finished = false, finishedToday = false;
        int phase = 0;
        QString phaseTitle, dayKind, dayTitle, dayTitleEs;
        int minutes = 0, inputMinutes = 0;
        QVariantList tasks;
        int streak = 0, longestStreak = 0, freezes = 0;
        bool todayDone = false;
        QVariantList week;
        int doneCount = 0, paceDiff = 0;
        QString paceText, targetText;
        QVariantList phases, calendar;
        QString calendarTitle;
        QVariantList coach;
    };

    LearnerController *m_learner;
    View m_view;
};
