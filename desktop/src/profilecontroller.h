#pragma once

#include "lq/database.h"
#include "lq/profile.h"

#include <QDate>
#include <QObject>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

#include <optional>

// The learner profile for QML: whether the welcome flow is done, and saving what it collects.
class ProfileController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by the application")
    Q_PROPERTY(bool exists READ exists NOTIFY changed FINAL)
    Q_PROPERTY(QString name READ name NOTIFY changed FINAL)
    Q_PROPERTY(QString reason READ reason NOTIFY changed FINAL)
    Q_PROPERTY(QString streakEmoji READ streakEmoji NOTIFY changed FINAL)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged FINAL)
    Q_PROPERTY(int maxNameLength READ maxNameLength CONSTANT FINAL)
    Q_PROPERTY(int minReasonLength READ minReasonLength CONSTANT FINAL)
    Q_PROPERTY(int maxReasonLength READ maxReasonLength CONSTANT FINAL)

public:
    explicit ProfileController(QDate today, QObject *parent = nullptr);

    // Loads the stored profile; later saves go to the same database.
    void attach(lq::Database *database);
    // Shows a profile without storing anything (the sample learner).
    void preview(const lq::Profile &profile);
    // Reads the stored profile again, after a restored backup replaced it.
    void reload();
    void setError(const QString &error);

    const std::optional<lq::Profile> &profile() const { return m_profile; }
    bool exists() const { return m_profile.has_value(); }
    QString name() const { return m_profile ? m_profile->name : QString(); }
    QString reason() const { return m_profile ? m_profile->reason : QString(); }
    QString streakEmoji() const { return m_profile ? m_profile->streakEmoji : QString(); }
    QString error() const { return m_error; }
    int maxNameLength() const { return lq::kMaxNameLength; }
    int minReasonLength() const { return lq::kMinReasonLength; }
    int maxReasonLength() const { return lq::kMaxReasonLength; }

    // Plan length suggested for "A1", "A2" or "A2+".
    Q_INVOKABLE int recommendedPlanDays(const QString &level) const;
    // Validates and stores the welcome-flow answers. On failure, error says why.
    Q_INVOKABLE bool save(const QString &name, const QString &reason, const QStringList &goals, const QString &level,
                          int planDays);

    // Shows the first emoji of `text` next to the streak; empty text brings back the app's flame. False when
    // `text` does not start with an emoji, or when saving fails (error then says why).
    Q_INVOKABLE bool setStreakEmoji(const QString &text);

    // The sample learner shown with --demo: Cory, three weeks into a 60-day plan.
    static lq::Profile demoProfile(QDate today);

signals:
    void changed();
    void errorChanged();

private:
    QDate m_today;
    lq::Database *m_database = nullptr;
    bool m_previewing = false;
    std::optional<lq::Profile> m_profile;
    QString m_error;
};
