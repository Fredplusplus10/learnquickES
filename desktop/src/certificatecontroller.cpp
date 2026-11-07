#include "certificatecontroller.h"

#include "profilecontroller.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QJSEngine>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QPointer>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QSvgRenderer>
#include <QVariantMap>

#include <type_traits>

static_assert(!std::is_default_constructible_v<CertificateController>, "QML would make its own instance");

namespace {

QPointer<CertificateController> s_instance;

QString levelTitle(const QString &level)
{
    if (level == QStringLiteral("A1"))
        return QStringLiteral("Foundations for everyday Spanish");
    if (level == QStringLiteral("A2"))
        return QStringLiteral("Everyday conversations");
    return QStringLiteral("Functional, independent Spanish");
}

QString writeFile(const QString &path, const QByteArray &bytes)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return QStringLiteral("%1 could not be written: %2").arg(QDir::toNativeSeparators(path), file.errorString());
    file.write(bytes);
    if (!file.commit())
        return QStringLiteral("%1 could not be written: %2").arg(QDir::toNativeSeparators(path), file.errorString());
    return {};
}

// Qt's SVG renderer takes a font-family list ("Georgia, Times New Roman, serif") as one unknown name and
// falls back to the default sans. For drawing in the app and in the PDF, each list becomes its first
// installed family; the SVG the learner saves keeps the template's lists, which browsers read.
QByteArray forQtRendering(const QByteArray &svg)
{
    static const QRegularExpression familyList(QStringLiteral("font-family=\"([^\"]*,[^\"]*)\""));
    QString text = QString::fromUtf8(svg);
    QString result;
    qsizetype last = 0;
    for (const QRegularExpressionMatch &match : familyList.globalMatch(text)) {
        const QStringList families = match.captured(1).split(u',');
        QString chosen;
        for (QString family : families) {
            family = family.trimmed().remove(u'"').remove(u'\x27');
            if (family == QStringLiteral("serif") || family == QStringLiteral("sans-serif"))
                break;  // a generic fallback: Qt's default font stands in for it
            if (QFontDatabase::hasFamily(family)) {
                chosen = family;
                break;
            }
        }
        if (chosen.isEmpty())
            continue;
        result += QStringView(text).mid(last, match.capturedStart() - last);
        result += QStringLiteral("font-family=\"%1\"").arg(chosen.toHtmlEscaped());
        last = match.capturedEnd();
    }
    result += QStringView(text).mid(last);
    return result.toUtf8();
}

} // namespace

void CertificateController::setInstance(CertificateController *instance)
{
    s_instance = instance;
}

CertificateController *CertificateController::create(QQmlEngine *, QJSEngine *engine)
{
    if (!s_instance)
        return new CertificateController(nullptr);  // owned by the engine
    Q_ASSERT(engine->thread() == s_instance->thread());
    QJSEngine::setObjectOwnership(s_instance, QJSEngine::CppOwnership);
    return s_instance;
}

CertificateController::CertificateController(QObject *parent)
    : QObject(parent)
{
}

void CertificateController::attach(lq::Database *database, const QString &dataDir)
{
    m_database = database;
    m_dataDir = dataDir;
    m_profileId = database ? lq::profileId(database->connection()) : QString();
    reload();
}

void CertificateController::setProfile(const ProfileController *profile)
{
    if (m_profile)
        disconnect(m_profile, nullptr, this, nullptr);
    m_profile = profile;
    if (m_profile)
        connect(m_profile, &ProfileController::changed, this, &CertificateController::changed);
    emit changed();
}

void CertificateController::reload()
{
    m_awards = m_database && !m_profileId.isEmpty() ? lq::loadCertificates(m_database->connection(), m_profileId)
                                                    : QList<lq::CertificateAward>{};
    emit changed();
}

QString CertificateController::learnerName() const
{
    return m_profile ? m_profile->name() : QString();
}

QString CertificateController::disclaimer() const
{
    return QStringLiteral("LearnQuick learning-readiness certificate. Not an official CEFR or DELE qualification "
                          "or examination certificate.");
}

const lq::CertificateAward *CertificateController::award(const QString &level) const
{
    // A learner keeps every award; the newest course revision's comes last.
    const lq::CertificateAward *found = nullptr;
    for (const lq::CertificateAward &a : m_awards) {
        if (a.level == level)
            found = &a;
    }
    return found;
}

QString CertificateController::filePath(const lq::CertificateAward &award) const
{
    return QDir(m_dataDir).filePath(QStringLiteral("certificates/%1.svg").arg(award.certificateId));
}

QString CertificateController::previewPath(const lq::CertificateAward &award) const
{
    return QDir(m_dataDir).filePath(QStringLiteral("certificates/%1.preview.svg").arg(award.certificateId));
}

QByteArray CertificateController::certificateSvg(const lq::CertificateAward &award, QString *error) const
{
    const QString path = filePath(award);
    if (QFile existing(path); existing.open(QIODevice::ReadOnly))
        return existing.readAll();
    QFile master(QDir(m_templateDir).filePath(QStringLiteral("certificate_%1.svg").arg(award.level)));
    if (!master.open(QIODevice::ReadOnly)) {
        if (error)
            *error = QStringLiteral("The %1 certificate template is missing from the app.").arg(award.level);
        return {};
    }
    // From the stored record: the name as it was issued, the original date and ID.
    const QByteArray svg = lq::personalizeCertificate(master.readAll(), award.usernameAtIssue, award.issuedOn,
                                                      award.certificateId, error);
    if (svg.isEmpty())
        return {};
    if (QDir().mkpath(QFileInfo(path).absolutePath()))
        writeFile(path, svg);  // a cache: the record in the database is what counts
    return svg;
}

QVariantList CertificateController::levels() const
{
    QVariantList list;
    for (const QString &level : lq::kCertificateLevels) {
        QVariantMap entry{
            {QStringLiteral("level"), level},
            {QStringLiteral("title"), levelTitle(level)},
            {QStringLiteral("earned"), false},
            {QStringLiteral("certificateId"), QString()},
            {QStringLiteral("issuedOn"), QString()},
            {QStringLiteral("username"), QString()},
            {QStringLiteral("file"), QUrl()},
            {QStringLiteral("lockedReason"), QString()},
            {QStringLiteral("assessmentOpen"), false},
            {QStringLiteral("ready"), false},
            {QStringLiteral("lessonsDone"), 0},
            {QStringLiteral("lessonsNeeded"), lq::assessmentUnits(level.toStdString())},
            {QStringLiteral("attempts"), 0},
            {QStringLiteral("bestPercent"), -1},
        };
        if (const lq::CertificateAward *a = award(level)) {
            QString error;
            const QByteArray svg = certificateSvg(*a, &error);
            // The copy the app draws, with fonts Qt can find (see forQtRendering).
            const bool hasFile = !svg.isEmpty() && writeFile(previewPath(*a), forQtRendering(svg)).isEmpty();
            entry[QStringLiteral("earned")] = true;
            entry[QStringLiteral("certificateId")] = a->certificateId;
            entry[QStringLiteral("issuedOn")] = a->issuedOn.toString(QStringLiteral("dd/MM/yyyy"));
            entry[QStringLiteral("username")] = a->usernameAtIssue;
            entry[QStringLiteral("file")] = hasFile ? QUrl::fromLocalFile(previewPath(*a)) : QUrl();
        } else if (!m_database) {
            entry[QStringLiteral("lockedReason")] = QStringLiteral("The sample learner has no certificates.");
        } else {
            const lq::AssessmentAccess access = lq::assessmentAccess(m_database->connection(), m_profileId, level, m_today);
            // Passed but not issued yet (the name could not be printed, say): the reason is what issuing says.
            const bool ready = lq::readinessEvidence(m_database->connection(), m_profileId, level).eligible();
            entry[QStringLiteral("lockedReason")] = access.reason;
            entry[QStringLiteral("assessmentOpen")] = access.open;
            entry[QStringLiteral("ready")] = ready;
            entry[QStringLiteral("lessonsDone")] = access.lessons.done;
            entry[QStringLiteral("lessonsNeeded")] = access.lessons.needed;
            entry[QStringLiteral("attempts")] = access.attempts;
            entry[QStringLiteral("bestPercent")] = access.bestPercent;
        }
        list.append(entry);
    }
    return list;
}

QString CertificateController::issue(const QString &level)
{
    if (!m_database)
        return QStringLiteral("The sample learner can't receive certificates.");
    if (!m_profile || !m_profile->exists())
        return QStringLiteral("There is no learner profile yet.");
    // The only evidence the app uses: the level's own assessment, never days, lessons or checkpoints alone.
    const lq::ReadinessEvidence evidence = lq::readinessEvidence(m_database->connection(), m_profileId, level);
    if (!lq::kCertificateIssuanceEnabled)
        return evidence.reason;
    const lq::IssueResult result = lq::issueCertificate(m_database->connection(), m_profileId, m_profile->name(),
                                                        level, evidence, m_today);
    if (!result.award)
        return result.error;
    reload();
    if (result.created)
        emit issued(level);
    return {};
}

QVariantMap CertificateController::finishAssessment(const QString &level, const QVariantMap &scores)
{
    const auto tally = [&scores](const char *key) {
        const QVariantList pair = scores.value(QString::fromLatin1(key)).toList();
        return pair.size() == 2 ? lq::Tally{pair[0].toInt(), pair[1].toInt()} : lq::Tally{};
    };
    const lq::AssessmentScores s{tally("reading"), tally("listening"), tally("language")};
    const lq::AssessmentResult r = lq::assessmentResult(s);
    QVariantMap out{
        {QStringLiteral("level"), level},
        {QStringLiteral("reading"), r.reading},
        {QStringLiteral("listening"), r.listening},
        {QStringLiteral("language"), r.language},
        {QStringLiteral("overall"), r.overall},
        {QStringLiteral("overallPassed"), r.overallPassed},
        {QStringLiteral("partsPassed"), r.partsPassed},
        {QStringLiteral("pass"), r.pass()},
        {QStringLiteral("saved"), false},
        {QStringLiteral("issued"), false},
        {QStringLiteral("error"), QString()},
    };
    if (!m_database) {
        out[QStringLiteral("error")] = QStringLiteral("The sample learner's assessments are not saved.");
        return out;
    }
    QString error;
    if (!lq::saveAssessment(m_database->connection(), m_profileId, level, s, m_today, &error)) {
        out[QStringLiteral("error")] = error;
        return out;
    }
    out[QStringLiteral("saved")] = true;
    if (r.pass()) {
        // Issued at once: the certificate rests on the attempt just stored.
        const QString problem = issue(level);
        out[QStringLiteral("issued")] = problem.isEmpty();
        out[QStringLiteral("error")] = problem;
    }
    emit changed();
    return out;
}

QString CertificateController::assessmentSeed(const QString &level) const
{
    return QStringLiteral("%1/%2/%3").arg(m_profileId, level, m_today.toString(Qt::ISODate));
}

QUrl CertificateController::suggestedFile(const QString &level, const QString &extension) const
{
    const lq::CertificateAward *a = award(level);
    QString folder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (folder.isEmpty())
        folder = QDir::homePath();
    const QString name = a ? QStringLiteral("LearnQuick-%1-certificate-%2.%3").arg(level, a->certificateId, extension)
                           : QStringLiteral("LearnQuick-%1-certificate.%2").arg(level, extension);
    return QUrl::fromLocalFile(QDir(folder).filePath(name));
}

QString CertificateController::saveSvg(const QString &level, const QUrl &file)
{
    const lq::CertificateAward *a = award(level);
    if (!a)
        return QStringLiteral("You have not earned the %1 certificate yet.").arg(level);
    if (!file.isLocalFile())
        return QStringLiteral("Choose a file on this computer.");
    QString error;
    const QByteArray svg = certificateSvg(*a, &error);
    if (svg.isEmpty())
        return error;
    return writeFile(file.toLocalFile(), svg);
}

QString CertificateController::exportPdf(const QString &level, const QUrl &file)
{
    const lq::CertificateAward *a = award(level);
    if (!a)
        return QStringLiteral("You have not earned the %1 certificate yet.").arg(level);
    if (!file.isLocalFile())
        return QStringLiteral("Choose a file on this computer.");
    QString error;
    const QByteArray svg = certificateSvg(*a, &error);
    if (svg.isEmpty())
        return error;
    QSvgRenderer renderer(forQtRendering(svg));
    if (!renderer.isValid())
        return QStringLiteral("The certificate could not be drawn.");

    const QString path = file.toLocalFile();
    const QString shownPath = QDir::toNativeSeparators(path);
    // Written beside the target and renamed at the end, like the SVG: a failed export never leaves half a PDF in
    // place of the one already there.
    QSaveFile out(path);
    if (!out.open(QIODevice::WriteOnly))
        return QStringLiteral("%1 could not be written: %2").arg(shownPath, out.errorString());
    {
        QPdfWriter pdf(&out);
        pdf.setTitle(QStringLiteral("LearnQuick %1 Certificate of Readiness · %2").arg(level, a->certificateId));
        pdf.setCreator(QStringLiteral("LearnQuick"));
        pdf.setResolution(300);
        pdf.setPageLayout(QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Landscape, QMarginsF(0, 0, 0, 0)));
        QPainter painter;
        if (!painter.begin(&pdf))
            return QStringLiteral("%1 could not be written.").arg(shownPath);
        // The template is A4 landscape too (297 × 210 mm), so it fills the page edge to edge.
        renderer.setAspectRatioMode(Qt::KeepAspectRatio);
        renderer.render(&painter, QRectF(0, 0, pdf.width(), pdf.height()));
        painter.end();
    }
    if (out.size() <= 0)
        return QStringLiteral("%1 could not be written.").arg(shownPath);
    if (!out.commit())
        return QStringLiteral("%1 could not be written: %2").arg(shownPath, out.errorString());
    return {};
}
