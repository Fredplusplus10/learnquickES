#pragma once

#include <QObject>
#include <QString>
#include <QThreadPool>
#include <QtQml/qqmlregistration.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class QAudioSink;
class QJSEngine;
class QQmlEngine;
class QTextToSpeech;
namespace lq {
class SupertonicVoice;
}

// Spanish speech for the whole app. It speaks with the neural voice bundled with the app (Supertonic 2, the
// same on Windows and macOS, with a voice for each dialogue character) and falls back to the voices the
// operating system has (Qt TextToSpeech: AVSpeech on macOS, WinRT or SAPI on Windows) when the bundled voice
// is missing or fails. Either way, symbols and numbers are read as a teacher would ("cansado / cansada" →
// "cansado o cansada", "305" → "trescientos cinco").
// QML sees it as the Speech singleton, so any button can speak without being handed the controller.
class SpeechController : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Speech)
    QML_SINGLETON
    Q_PROPERTY(bool available READ available NOTIFY availableChanged FINAL)
    Q_PROPERTY(bool speaking READ speaking NOTIFY speakingChanged FINAL)
    Q_PROPERTY(QString voiceName READ voiceName NOTIFY availableChanged FINAL)

public:
    // No default constructor: QML would prefer it to create() and build a second instance of its own, cut
    // off from the one main() sets up.
    explicit SpeechController(QObject *parent);
    ~SpeechController() override;

    bool available() const { return m_available; }
    bool speaking() const { return m_speaking; }
    QString voiceName() const { return m_voiceName; }

    // slow: about three quarters of the normal pace, for listening again. voice: one of the bundled voice's
    // styles, as lq::voiceFor gives for a dialogue speaker; empty, or a style it lacks, is the narrator.
    // The system's voices ignore it.
    Q_INVOKABLE void say(const QString &text, bool slow = false, const QString &voice = {});
    Q_INVOKABLE void stop();

    // Muted, say() does nothing: smoke tests and screenshots open screens that speak on their own.
    void setMuted(bool muted) { m_muted = muted; }
    // The learner's own voice from Settings, for everything outside a dialogue, and the bundled voice's two
    // paces (how many times longer than its natural pace it speaks; see lq::kDefaultPace).
    void setNarrator(const QString &style) { m_narrator = style.toStdString(); }
    void setPace(double pace, double slowPace);
    // Loads the bundled voice in the background now, so the first phrase is not delayed by it.
    void preload();
    // Loads the bundled voice and speaks a word into memory; false, with the reason, when it cannot.
    bool checkBundledVoice(QString *error) const;

    // The folder with the bundled voice's model; empty when the app was built or deployed without it.
    static QString bundledVoicePath();
    // The operating system's speech engines the app can load; empty when none was deployed with it.
    static QStringList engines();

    // The instance QML gets as Speech; set before the engine loads.
    static void setInstance(SpeechController *instance);
    static SpeechController *create(QQmlEngine *, QJSEngine *engine);

signals:
    void availableChanged();
    void speakingChanged();
    // The last phrase finished playing (not sent after stop()).
    void finished();

private:
    void startSystemVoice();
    void chooseVoice();
    void setSpeaking(bool speaking);
    void sayWithSystemVoice(const QString &text, bool slow);
    // Runs on the worker thread: loads the voice the first time.
    bool ensureVoice(QString *error);
    void play(quint64 request, const std::vector<std::int16_t> &pcm, int sampleRate);
    void bundledVoiceFailed(const QString &error, quint64 request, const QString &text, bool slow);
    void stopAudio();

    // The bundled voice.
    QString m_voicePath;
    bool m_neural = false;
    std::shared_ptr<lq::SupertonicVoice> m_voice;  // created and used on the worker thread only
    QThreadPool m_worker;
    std::atomic<quint64> m_request = 0;  // the phrase being spoken; the worker drops anything older
    QAudioSink *m_sink = nullptr;  // owns the buffer it plays

    // The operating system's voice.
    QTextToSpeech *m_tts = nullptr;

    bool m_available = false;
    bool m_speaking = false;
    bool m_stopped = false;
    bool m_muted = false;
    QString m_voiceName;
    std::string m_narrator;
    float m_pace = 1.0f;
    float m_slowPace = 1.0f;
};
