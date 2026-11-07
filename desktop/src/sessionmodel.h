#pragma once

#include "learnercontroller.h"
#include "lq/assessment.h"
#include "lq/exercises.h"
#include "lq/progress.h"

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

// One exercise session on screen: a lesson's practice step, a trainer, the review, a checkpoint or a part of
// the B1 mock. The engine's Session decides the order and the scoring; this adds what the learner is doing
// on the current item (the option picked, the tiles placed, the pairs matched) and records every answer.
class SessionModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by the application")
    Q_PROPERTY(bool active READ active NOTIFY changed FINAL)
    Q_PROPERTY(bool finished READ finished NOTIFY changed FINAL)
    Q_PROPERTY(QString kind READ kind NOTIFY changed FINAL)
    Q_PROPERTY(int position READ position NOTIFY changed FINAL)
    Q_PROPERTY(int count READ count NOTIFY changed FINAL)
    Q_PROPERTY(bool secondTry READ secondTry NOTIFY changed FINAL)
    Q_PROPERTY(bool intermission READ intermission NOTIFY changed FINAL)
    Q_PROPERTY(int secondTryCount READ secondTryCount NOTIFY changed FINAL)
    Q_PROPERTY(int secondTryXp READ secondTryXp CONSTANT FINAL)
    Q_PROPERTY(QVariantMap item READ item NOTIFY changed FINAL)
    Q_PROPERTY(bool checked READ checked NOTIFY changed FINAL)
    Q_PROPERTY(bool canCheck READ canCheck NOTIFY changed FINAL)
    Q_PROPERTY(int selected READ selected NOTIFY changed FINAL)
    Q_PROPERTY(QString chosenArticle READ chosenArticle NOTIFY changed FINAL)
    Q_PROPERTY(QVariantList picked READ picked NOTIFY changed FINAL)
    Q_PROPERTY(QVariantList matched READ matched NOTIFY changed FINAL)
    Q_PROPERTY(QString matchSide READ matchSide NOTIFY changed FINAL)
    Q_PROPERTY(int matchIndex READ matchIndex NOTIFY changed FINAL)
    Q_PROPERTY(QVariantMap matchMiss READ matchMiss NOTIFY changed FINAL)
    Q_PROPERTY(bool revealed READ revealed NOTIFY changed FINAL)
    Q_PROPERTY(QVariantMap feedback READ feedback NOTIFY changed FINAL)
    Q_PROPERTY(QVariantMap result READ result NOTIFY changed FINAL)

public:
    // seed: 0 draws a random one; tests pass a fixed value for reproducible sessions.
    explicit SessionModel(LearnerController *learner, quint32 seed = 0, QObject *parent = nullptr);

    // Without a Spanish voice, listening items are asked as sentence building instead.
    void setAudioAvailable(bool available);

    bool active() const { return m_active; }
    bool finished() const { return m_active && m_session.finished(); }
    QString kind() const { return m_kind; }
    // Within the current round: the first one, or the second-try round of the first misses.
    int position() const { return static_cast<int>(m_session.roundPosition()); }
    int count() const { return static_cast<int>(m_session.roundSize()); }
    bool secondTry() const { return m_active && m_session.secondTry(); }
    // The first round is over and the second-try round is waiting for the learner to start it.
    bool intermission() const { return m_intermission; }
    int secondTryCount() const { return m_session.secondTry() ? count() : 0; }
    int secondTryXp() const { return lq::kXpRetry; }
    QVariantMap item() const { return m_item; }
    bool checked() const { return m_checked; }
    bool canCheck() const;
    int selected() const { return m_response.choice; }
    QString chosenArticle() const { return m_article; }
    QVariantList picked() const;
    QVariantList matched() const;
    QString matchSide() const { return m_matchSide; }
    int matchIndex() const { return m_matchIndex; }
    QVariantMap matchMiss() const { return m_matchMiss; }
    bool revealed() const { return m_revealed; }
    QVariantMap feedback() const { return m_feedback; }
    QVariantMap result() const { return m_result; }

    Q_INVOKABLE void startLesson(int unit);
    // "review", "verbs", "mixed", "speak", "cognates", "falsefriends", "gender", "past" or "describe".
    Q_INVOKABLE void startTrainer(const QString &id);
    Q_INVOKABLE void startCheckpoint(int phase);
    // "reading", "listening" or "grammar".
    Q_INVOKABLE void startMockPart(const QString &part);
    // A level assessment (lq/assessment.h): draws this attempt's texts and questions for "A1", "A2" or "B1" and
    // returns level, readingText, listeningText, listeningIntro, and reading, listening and language (the number
    // of questions in each part). Empty when there is no such assessment. The draw follows `seed`
    // (Certificates.assessmentSeed), so starting again the same day brings back the same texts and questions.
    Q_INVOKABLE QVariantMap startAssessment(const QString &level, const QString &seed);
    // "reading", "listening" or "language", from the draw startAssessment made. It is a test, not practice: no
    // answer is shown or counted towards XP and skills, each one moves straight on, and there are no second tries.
    Q_INVOKABLE void startAssessmentPart(const QString &part);
    Q_INVOKABLE void stop();

    Q_INVOKABLE void select(int option);
    Q_INVOKABLE void pick(int tile);
    Q_INVOKABLE void unpick(int tile);
    Q_INVOKABLE void setInput(const QString &text);
    Q_INVOKABLE void check();
    Q_INVOKABLE void skip();  // "I don't know"
    Q_INVOKABLE void next();
    Q_INVOKABLE void reveal();
    Q_INVOKABLE void chooseArticle(const QString &article);
    Q_INVOKABLE void tapMatch(const QString &side, int index);
    Q_INVOKABLE void selfRate(bool managed);  // speaking and describing
    Q_INVOKABLE void grade(int grade);        // review cards: 0 again, 2 good, 3 easy
    Q_INVOKABLE void startSecondTry();
    Q_INVOKABLE void skipSecondTry();

signals:
    void changed();
    // Session over; result is ready.
    void completed();
    // Spanish worth hearing now, e.g. a pair just matched.
    void sayRequested(const QString &text);

private:
    void begin(const QString &kind, const QString &trainer, std::vector<lq::Item> items, bool requeue);
    void startItem();
    void advance();
    bool onItem() const { return m_active && !m_session.finished() && !m_intermission; }
    void answer(const lq::AnswerCheck &check);
    void finish();
    QVariantMap itemMap(const lq::Item &item) const;

    LearnerController *m_learner;
    lq::SeededRandom m_rnd;
    lq::Session m_session;
    bool m_audio = true;
    bool m_active = false;
    bool m_intermission = false;
    QString m_kind;     // "lesson", "trainer", "checkpoint", "mock" or "assessment"
    QString m_trainer;  // trainer id, unit index or phase, depending on the kind
    QStringList m_kinds;  // exercise kinds in the session, for the reflection
    int m_xpAtStart = 0;
    lq::AssessmentDraw m_draw;  // the level assessment being taken

    // The current item and what the learner has done on it.
    QVariantMap m_item;
    lq::Response m_response;
    QString m_article;
    bool m_checked = false;
    bool m_revealed = false;
    QString m_matchSide;
    int m_matchIndex = -1;
    QVariantMap m_matchMiss;
    std::vector<int> m_matched;
    int m_matchMistakes = 0;
    QVariantMap m_feedback;
    QVariantMap m_result;
};
