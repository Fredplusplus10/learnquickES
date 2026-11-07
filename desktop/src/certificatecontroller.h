#pragma once

#include "lq/certificates.h"
#include "lq/database.h"

#include <QDate>
#include <QList>
#include <QObject>
#include <QUrl>
#include <QVariantMap>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

class ProfileController;
class QJSEngine;
class QQmlEngine;

// The A1, A2 and B1 readiness certificates for QML, as the Certificates singleton: which ones the learner has,
// and viewing, saving and exporting them. Certificates carry the name the learner chose on the welcome screen
// (never asked again) and are issued only through lq::readinessEvidence: the level's lessons done and a stored,
// passed attempt at its assessment (lq/assessment.h), which the Assessment screen takes and hands here.
class CertificateController : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Certificates)
    QML_SINGLETON
    Q_PROPERTY(QVariantList levels READ levels NOTIFY changed FINAL)
    Q_PROPERTY(QString learnerName READ learnerName NOTIFY changed FINAL)
    Q_PROPERTY(bool issuingEnabled READ issuingEnabled CONSTANT FINAL)
    Q_PROPERTY(bool stored READ stored NOTIFY changed FINAL)
    Q_PROPERTY(QString disclaimer READ disclaimer CONSTANT FINAL)

public:
    explicit CertificateController(QObject *parent);

    // The learner's database and the folder it lives in; personalised certificates are kept in its
    // "certificates" subfolder. Without it (the sample learner) nothing is stored or issued.
    void attach(lq::Database *database, const QString &dataDir);
    void setProfile(const ProfileController *profile);
    void setToday(QDate today) { m_today = today; }
    // Where the master templates are: the app's resources by default, the source tree in tests.
    void setTemplateDir(const QString &dir) { m_templateDir = dir; }

    // For each level: level, title, earned, certificateId, issuedOn (DD/MM/YYYY), username (as issued),
    // file (the URL of the copy the app draws, when earned), lockedReason (why it is not earned yet),
    // assessmentOpen (it can be taken today), ready (passed, waiting to be issued), lessonsDone, lessonsNeeded,
    // attempts and bestPercent (-1 before the first attempt).
    QVariantList levels() const;
    QString learnerName() const;
    bool issuingEnabled() const { return lq::kCertificateIssuanceEnabled; }
    bool stored() const { return m_database != nullptr; }
    QString disclaimer() const;
    const QList<lq::CertificateAward> &awards() const { return m_awards; }

    // Issues `level` if its assessment says the learner is ready; "" when it worked, otherwise why not.
    Q_INVOKABLE QString issue(const QString &level);
    // Marks and stores an attempt at `level`'s assessment, and issues the certificate when it passed. scores:
    // reading, listening and language as [right, asked]. Returns the percentages (reading, listening, language,
    // overall), overallPassed, partsPassed, pass, saved, issued and error ("" when nothing went wrong).
    Q_INVOKABLE QVariantMap finishAssessment(const QString &level, const QVariantMap &scores);
    // What today's draw of `level`'s assessment follows: the same all day for this learner, another tomorrow.
    Q_INVOKABLE QString assessmentSeed(const QString &level) const;
    // Documents/LearnQuick-<level>-certificate-<id>.<extension>
    Q_INVOKABLE QUrl suggestedFile(const QString &level, const QString &extension) const;
    // Copies an earned certificate to `file`. Both return "" when they worked, otherwise what went wrong.
    Q_INVOKABLE QString saveSvg(const QString &level, const QUrl &file);
    // An A4 landscape PDF of an earned certificate, drawn from its vectors.
    Q_INVOKABLE QString exportPdf(const QString &level, const QUrl &file);

    // The instance QML gets as Certificates; set before the engine loads.
    static void setInstance(CertificateController *instance);
    static CertificateController *create(QQmlEngine *, QJSEngine *engine);

signals:
    void changed();
    // A new award, for the congratulations.
    void issued(const QString &level);

private:
    void reload();
    const lq::CertificateAward *award(const QString &level) const;
    QString filePath(const lq::CertificateAward &award) const;
    QString previewPath(const lq::CertificateAward &award) const;
    // The personalised SVG of an award, rebuilt from its stored record when the file is missing.
    QByteArray certificateSvg(const lq::CertificateAward &award, QString *error) const;

    lq::Database *m_database = nullptr;
    const ProfileController *m_profile = nullptr;
    QString m_dataDir;
    QString m_profileId;
    QString m_templateDir = QStringLiteral(":/qt/qml/LearnQuick/assets/certificates");
    QDate m_today = QDate::currentDate();
    QList<lq::CertificateAward> m_awards;
};
