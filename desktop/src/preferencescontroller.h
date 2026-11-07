#pragma once

#include "lq/database.h"
#include "lq/settings.h"

#include <QObject>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

class QJSEngine;
class QQmlEngine;

// The Settings screen's choices for QML: day or night, the learner's own voice and its two paces. Every
// change is saved straight away. QML sees it as the Preferences singleton, so Theme can follow it.
class PreferencesController : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Preferences)
    QML_SINGLETON
    Q_PROPERTY(QString theme READ theme NOTIFY changed FINAL)
    Q_PROPERTY(QString voice READ voice NOTIFY changed FINAL)
    Q_PROPERTY(double pace READ pace NOTIFY changed FINAL)
    Q_PROPERTY(double slowPace READ slowPace NOTIFY changed FINAL)
    Q_PROPERTY(bool recommendedVoice READ recommendedVoice NOTIFY changed FINAL)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged FINAL)
    Q_PROPERTY(QVariantList voices READ voices CONSTANT FINAL)
    Q_PROPERTY(QString defaultVoice READ defaultVoice CONSTANT FINAL)
    Q_PROPERTY(double defaultPace READ defaultPace CONSTANT FINAL)
    Q_PROPERTY(double minPace READ minPace CONSTANT FINAL)
    Q_PROPERTY(double maxPace READ maxPace CONSTANT FINAL)
    Q_PROPERTY(double defaultSlowPace READ defaultSlowPace CONSTANT FINAL)
    Q_PROPERTY(double minSlowPace READ minSlowPace CONSTANT FINAL)
    Q_PROPERTY(double maxSlowPace READ maxSlowPace CONSTANT FINAL)

public:
    // No default constructor: QML would prefer it to create() and build a second instance of its own, cut
    // off from the one main() sets up.
    explicit PreferencesController(QObject *parent);

    // Loads the stored settings; later changes are saved to the same database.
    void attach(lq::Database *database);
    // Shows these settings without storing anything (--demo, --theme).
    void preview(const lq::Settings &settings);

    const lq::Settings &settings() const { return m_settings; }
    QString theme() const { return m_settings.theme; }
    QString voice() const { return m_settings.voice; }
    double pace() const { return m_settings.pace; }
    double slowPace() const { return m_settings.slowPace; }
    // The voice and both paces are the ones the app recommends.
    bool recommendedVoice() const;
    QString error() const { return m_error; }
    // {style, name, detail} for every voice on offer, the clearest first.
    QVariantList voices() const;
    // The voice a new learner starts with, the one Settings recommends.
    QString defaultVoice() const { return lq::Settings{}.voice; }
    double defaultPace() const { return lq::kDefaultPace; }
    double minPace() const { return lq::kMinPace; }
    double maxPace() const { return lq::kMaxPace; }
    double defaultSlowPace() const { return lq::kDefaultSlowPace; }
    double minSlowPace() const { return lq::kMinSlowPace; }
    double maxSlowPace() const { return lq::kMaxSlowPace; }

    Q_INVOKABLE void setTheme(const QString &theme);
    Q_INVOKABLE void setVoice(const QString &style);
    Q_INVOKABLE void setPace(double pace);
    Q_INVOKABLE void setSlowPace(double pace);
    // The recommended voice and paces again; the theme stays.
    Q_INVOKABLE void resetVoice();
    // About how many syllables a second the bundled voice says at this pace.
    Q_INVOKABLE double syllablesPerSecond(double pace) const;

    // The instance QML gets as Preferences; set before the engine loads.
    static void setInstance(PreferencesController *instance);
    static PreferencesController *create(QQmlEngine *, QJSEngine *engine);

signals:
    void changed();
    void errorChanged();

private:
    void apply(const lq::Settings &settings);
    void setError(const QString &error);

    lq::Database *m_database = nullptr;
    lq::Settings m_settings;
    QString m_error;
};
