#include "speechcontroller.h"

#include "lq/speech.h"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QBuffer>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJSEngine>
#include <QMediaDevices>
#include <QPointer>
#include <QTextToSpeech>
#include <QVoice>

#ifdef LQ_NEURAL_VOICE
#include "lq/supertonic.h"

#include <filesystem>
#endif

#include <algorithm>
#include <cmath>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

static_assert(!std::is_default_constructible_v<SpeechController>, "QML would make its own instance");

namespace {

// Lower is better: Mexico, then the United States, then the rest of Latin America, then Spain.
int preference(const QLocale &locale)
{
    switch (locale.territory()) {
    case QLocale::Mexico: return 0;
    case QLocale::UnitedStates: return 1;
    case QLocale::Spain: return 3;
    default: return 2;
    }
}

constexpr double kNormalRate = -0.1;  // a touch slower than the system default, as the web app's 0.9
constexpr double kSlowRate = -0.45;

QPointer<SpeechController> s_instance;

QString spoken(const QString &text)
{
    return QString::fromStdString(lq::speechText(text.toStdString()));
}

// 16-bit mono at the voice's rate, as the output device takes it. Most devices accept it as it is; for the
// rest it is resampled linearly and copied to every channel.
QByteArray toDevice(const std::vector<std::int16_t> &pcm, int rate, const QAudioFormat &format)
{
    const int outRate = format.sampleRate();
    const int channels = format.channelCount();
    const auto frames = outRate == rate ? pcm.size()
                                        : static_cast<std::size_t>(static_cast<double>(pcm.size()) * outRate / rate);
    QByteArray out(static_cast<qsizetype>(frames * static_cast<std::size_t>(channels * format.bytesPerSample())),
                   Qt::Uninitialized);
    char *data = out.data();
    const double step = static_cast<double>(rate) / outRate;
    for (std::size_t i = 0; i < frames; ++i) {
        const double position = static_cast<double>(i) * step;
        const auto k = std::min(static_cast<std::size_t>(position), pcm.size() - 1);
        const auto next = std::min(k + 1, pcm.size() - 1);
        const double fraction = position - static_cast<double>(k);
        const float sample = static_cast<float>((pcm[k] * (1.0 - fraction) + pcm[next] * fraction) / 32768.0);
        for (int c = 0; c < channels; ++c) {
            switch (format.sampleFormat()) {
            case QAudioFormat::UInt8:
                *reinterpret_cast<quint8 *>(data) = static_cast<quint8>(std::lround(sample * 127.0f + 128.0f));
                break;
            case QAudioFormat::Int16:
                *reinterpret_cast<qint16 *>(data) = static_cast<qint16>(std::lround(sample * 32767.0f));
                break;
            case QAudioFormat::Int32:
                *reinterpret_cast<qint32 *>(data) = static_cast<qint32>(std::lround(sample * 2147483647.0));
                break;
            case QAudioFormat::Float:
            default:
                *reinterpret_cast<float *>(data) = sample;
                break;
            }
            data += format.bytesPerSample();
        }
    }
    return out;
}

} // namespace

void SpeechController::setInstance(SpeechController *instance)
{
    s_instance = instance;
}

SpeechController *SpeechController::create(QQmlEngine *, QJSEngine *engine)
{
    if (!s_instance)
        return new SpeechController(nullptr);  // owned by the engine
    Q_ASSERT(engine->thread() == s_instance->thread());
    QJSEngine::setObjectOwnership(s_instance, QJSEngine::CppOwnership);
    return s_instance;
}

SpeechController::SpeechController(QObject *parent)
    : QObject(parent)
    , m_narrator(lq::kNarratorVoice)
    , m_pace(static_cast<float>(lq::kDefaultPace))
    , m_slowPace(static_cast<float>(lq::kDefaultSlowPace))
{
    // One phrase at a time; inference needs more stack than the 512 KB macOS gives secondary threads.
    m_worker.setMaxThreadCount(1);
    m_worker.setStackSize(8 * 1024 * 1024);
    m_voicePath = bundledVoicePath();
    if (!m_voicePath.isEmpty()) {
        m_neural = true;
        m_available = true;
        m_voiceName = QStringLiteral("Supertonic, Spanish (built in)");
        return;
    }
    startSystemVoice();
}

SpeechController::~SpeechController()
{
    ++m_request;
    m_worker.clear();
    m_worker.waitForDone();
    delete std::exchange(m_sink, nullptr);
}

QString SpeechController::bundledVoicePath()
{
#ifdef LQ_NEURAL_VOICE
    // LQ_VOICE_DIR first, for development; then bin/voices on Windows and Contents/Resources/voices on macOS.
    QStringList dirs;
    if (const QString dir = qEnvironmentVariable("LQ_VOICE_DIR"); !dir.isEmpty())
        dirs << dir;
    const QString app = QCoreApplication::applicationDirPath();
    dirs << app + QStringLiteral("/voices") << app + QStringLiteral("/../Resources/voices");
    for (const QString &dir : std::as_const(dirs)) {
        const QDir folder(dir);
        if (QFileInfo::exists(folder.filePath(QStringLiteral("tts.json")))
            && QFileInfo::exists(folder.filePath(QStringLiteral("vector_estimator.onnx"))))
            return QDir::cleanPath(folder.absolutePath());
    }
#endif
    return {};
}

QStringList SpeechController::engines()
{
    return QTextToSpeech::availableEngines();
}

void SpeechController::startSystemVoice()
{
    if (m_tts || engines().isEmpty())
        return;
    m_tts = new QTextToSpeech(this);
    connect(m_tts, &QTextToSpeech::stateChanged, this, [this](QTextToSpeech::State state) {
        switch (state) {
        case QTextToSpeech::Ready:
            if (!m_available)
                chooseVoice();
            if (m_speaking) {
                setSpeaking(false);
                if (!m_stopped)
                    emit finished();
            }
            break;
        case QTextToSpeech::Speaking:
        case QTextToSpeech::Synthesizing:
            setSpeaking(true);
            break;
        case QTextToSpeech::Error:
            setSpeaking(false);
            break;
        case QTextToSpeech::Paused:
            break;
        }
    });
    if (m_tts->state() == QTextToSpeech::Ready)
        chooseVoice();
}

void SpeechController::chooseVoice()
{
    QList<QVoice> voices = m_tts->findVoices(QLocale::Spanish);
    if (voices.isEmpty())
        return;
    std::stable_sort(voices.begin(), voices.end(),
                     [](const QVoice &a, const QVoice &b) { return preference(a.locale()) < preference(b.locale()); });
    m_tts->setLocale(voices.first().locale());
    m_tts->setVoice(voices.first());
    m_voiceName = voices.first().name();
    m_available = true;
    emit availableChanged();
}

void SpeechController::setSpeaking(bool speaking)
{
    if (m_speaking == speaking)
        return;
    m_speaking = speaking;
    emit speakingChanged();
}

bool SpeechController::ensureVoice(QString *error)
{
#ifdef LQ_NEURAL_VOICE
    if (m_voice)
        return true;
    std::string reason;
    std::unique_ptr<lq::SupertonicVoice> voice
        = lq::SupertonicVoice::load(std::filesystem::path(m_voicePath.toStdU16String()), &reason);
    if (!voice) {
        *error = QString::fromStdString(reason);
        return false;
    }
    m_voice = std::move(voice);
    return true;
#else
    *error = QStringLiteral("built without the neural voice");
    return false;
#endif
}

void SpeechController::preload()
{
    if (!m_neural)
        return;
    const quint64 request = m_request;
    m_worker.start([this, request] {
        QString error;
        if (!ensureVoice(&error)) {
            QMetaObject::invokeMethod(this, [this, error, request] { bundledVoiceFailed(error, request, {}, false); },
                                      Qt::QueuedConnection);
        }
    });
}

bool SpeechController::checkBundledVoice(QString *error) const
{
#ifdef LQ_NEURAL_VOICE
    if (m_voicePath.isEmpty()) {
        *error = QStringLiteral("the voice files are not next to the app");
        return false;
    }
    std::string reason;
    const auto voice = lq::SupertonicVoice::load(std::filesystem::path(m_voicePath.toStdU16String()), &reason);
    if (!voice) {
        *error = QString::fromStdString(reason);
        return false;
    }
    for (const std::string_view style : {lq::kNarratorVoice, lq::voiceFor("Tú")}) {
        if (!voice->hasStyle(style)) {
            const QString name = QString::fromUtf8(style.data(), static_cast<qsizetype>(style.size()));
            *error = QStringLiteral("the voice %1 is missing").arg(name);
            return false;
        }
    }
    if (voice->synthesize(lq::speechText("Hola"), lq::kNarratorVoice).empty()) {
        *error = QStringLiteral("the voice loaded but produced no audio");
        return false;
    }
    return true;
#else
    *error = QStringLiteral("built without the neural voice");
    return false;
#endif
}

void SpeechController::setPace(double pace, double slowPace)
{
    m_pace = static_cast<float>(pace);
    m_slowPace = static_cast<float>(slowPace);
}

void SpeechController::say(const QString &text, bool slow, const QString &voice)
{
    if (m_muted || !m_available)
        return;
    if (!m_neural) {
        sayWithSystemVoice(text, slow);
        return;
    }
#ifdef LQ_NEURAL_VOICE
    const std::string words = lq::speechText(text.toStdString());
    if (words.empty())
        return;
    stopAudio();
    m_stopped = false;
    const quint64 request = ++m_request;
    setSpeaking(true);
    m_worker.clear();  // a phrase still waiting for its turn is no longer wanted
    const std::string style = voice.isEmpty() ? m_narrator : voice.toStdString();
    const float pace = slow ? m_slowPace : m_pace;
    m_worker.start([this, request, words, style, pace, text, slow] {
        if (request != m_request)
            return;
        QString error;
        if (!ensureVoice(&error)) {
            QMetaObject::invokeMethod(this, [=, this] { bundledVoiceFailed(error, request, text, slow); },
                                      Qt::QueuedConnection);
            return;
        }
        const std::string_view speaker = m_voice->hasStyle(style) ? std::string_view(style) : lq::kNarratorVoice;
        const double pause = slow ? lq::kSlowSentencePause : lq::kSentencePause;
        std::vector<std::int16_t> pcm = m_voice->synthesize(
            words, speaker, static_cast<float>(lq::voiceStretch(speaker, pace)), static_cast<float>(pause));
        const int rate = m_voice->sampleRate();
        QMetaObject::invokeMethod(this, [this, request, pcm = std::move(pcm), rate] { play(request, pcm, rate); },
                                  Qt::QueuedConnection);
    });
#endif
}

void SpeechController::sayWithSystemVoice(const QString &text, bool slow)
{
    const QString words = spoken(text);
    if (!m_tts || words.isEmpty())
        return;
    m_stopped = false;
    m_tts->setRate(slow ? kSlowRate : kNormalRate);
    m_tts->say(words);
}

void SpeechController::play(quint64 request, const std::vector<std::int16_t> &pcm, int sampleRate)
{
    if (request != m_request)
        return;  // stopped, or a newer phrase took its place
    const QAudioDevice device = QMediaDevices::defaultAudioOutput();
    if (pcm.empty() || device.isNull()) {
        setSpeaking(false);
        return;
    }
    QAudioFormat format;
    format.setSampleRate(sampleRate);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);
    if (!device.isFormatSupported(format))
        format = device.preferredFormat();

    stopAudio();
    m_sink = new QAudioSink(device, format, this);
    auto *audio = new QBuffer(m_sink);  // dies with the sink that reads it
    audio->setData(toDevice(pcm, sampleRate, format));
    audio->open(QIODevice::ReadOnly);
    connect(m_sink, &QAudioSink::stateChanged, this, [this, sink = m_sink](QAudio::State state) {
        if (sink != m_sink)
            return;
        // Idle: everything was played. Stopped with an error: the device went away.
        const bool done = state == QAudio::IdleState;
        if (done || (state == QAudio::StoppedState && sink->error() != QAudio::NoError)) {
            stopAudio();
            setSpeaking(false);
            if (done && !m_stopped)
                emit finished();
        }
    });
    m_sink->start(audio);
}

void SpeechController::bundledVoiceFailed(const QString &error, quint64 request, const QString &text, bool slow)
{
    if (m_neural) {
        qWarning("The built-in voice could not be used (%s); using the system's voices.", qUtf8Printable(error));
        m_neural = false;
        m_available = false;
        m_voiceName.clear();
        setSpeaking(false);
        startSystemVoice();
        emit availableChanged();
    }
    if (request == m_request && !text.isEmpty())
        sayWithSystemVoice(text, slow);
}

void SpeechController::stopAudio()
{
    // Called from the sink's own signal too, so it is deleted later rather than now.
    if (QAudioSink *sink = std::exchange(m_sink, nullptr)) {
        sink->stop();
        sink->deleteLater();
    }
}

void SpeechController::stop()
{
    m_stopped = true;
    if (m_tts)
        m_tts->stop(QTextToSpeech::BoundaryHint::Immediate);
    if (m_neural) {
        ++m_request;
        m_worker.clear();
        stopAudio();
        setSpeaking(false);
    }
}
