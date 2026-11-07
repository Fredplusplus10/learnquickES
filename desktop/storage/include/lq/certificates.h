#pragma once

#include <QByteArray>
#include <QDate>
#include <QList>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>

#include "lq/assessment.h"

#include <optional>

namespace lq {

// LearnQuick's internal A1, A2 and B1 readiness certificates: a learning record the app gives after that
// level's own assessment, never an official CEFR or DELE qualification. Each award is stored once and never
// changed: its certificate ID, level, the learner's name as it was that day, the course revision, the
// assessment it rests on and the date.

inline const QStringList kCertificateLevels{QStringLiteral("A1"), QStringLiteral("A2"), QStringLiteral("B1")};

// The course content an award was earned against. A new revision of the course allows a new award; the
// same revision never gives a second one.
inline const QString kCurriculumRevision = QStringLiteral("course-2026.1");

// On: each level has its own assessment (lq/assessment.h), and readinessEvidence reads its stored results.
// Lessons, days studied, phase checkpoints and the partly self-assessed B1 mock never earn a certificate.
inline constexpr bool kCertificateIssuanceEnabled = true;

inline constexpr int kMaxCertificateNameLength = 60;

// What a level's assessment says about one learner. Every flag must be true, with a reference to the
// assessment result, before a certificate can be issued.
struct ReadinessEvidence {
    QString level;
    bool curriculumComplete = false;
    bool assessmentPassed = false;
    bool skillsVerified = false;
    QString assessmentReference;  // the stored assessment result the award rests on
    QString reason;               // why it is not ready yet, in words for the learner

    bool eligible() const
    {
        return curriculumComplete && assessmentPassed && skillsVerified && !assessmentReference.isEmpty();
    }
};

// One attempt at a level assessment, as stored (schema 10). Written once, never changed or removed.
struct AssessmentRecord {
    QString reference;  // "assessment-<uuid>": what an award's assessment_reference points to
    QString profileId;
    QString level;
    QString curriculumRevision;
    QDate takenOn;
    lq::AssessmentScores scores;

    bool passed() const { return lq::assessmentResult(scores).pass(); }
};

// The learner's attempts at `level` against the current course revision, oldest first.
QList<AssessmentRecord> loadAssessments(const QSqlDatabase &db, const QString &profileId, const QString &level);

// How many of the lessons `level` needs are done (unit_result rows), and how many it needs.
struct LessonProgress {
    int done = 0;
    int needed = 0;
    bool complete() const { return needed > 0 && done >= needed; }
};
LessonProgress lessonProgress(const QSqlDatabase &db, const QString &level);

// Whether the learner may start `level`'s assessment today, and if not, why not in words for the learner.
// It opens when the level's lessons are done, once a day; once passed it is not taken again.
struct AssessmentAccess {
    bool open = false;
    QString reason;
    LessonProgress lessons;
    int attempts = 0;
    int bestPercent = -1;  // overall, of the attempts so far; -1 before the first
    bool passed = false;
};
AssessmentAccess assessmentAccess(const QSqlDatabase &db, const QString &profileId, const QString &level, QDate today);

// Stores an attempt the learner may take now (see assessmentAccess) and whose parts asked what an attempt
// asks. Returns the stored record, or nothing with the reason in `error`.
std::optional<AssessmentRecord> saveAssessment(const QSqlDatabase &db, const QString &profileId, const QString &level,
                                               const lq::AssessmentScores &scores, QDate today, QString *error = nullptr);

// The app's real evidence for `level`: the level's lessons done, and a stored attempt for this course revision
// that passed overall (assessmentPassed) and in every part (skillsVerified). The first such attempt is the
// reference. When it is not ready, `reason` says what is missing.
ReadinessEvidence readinessEvidence(const QSqlDatabase &db, const QString &profileId, const QString &level);

struct CertificateAward {
    QString certificateId;  // LQ-A1-2026-0E89AB2F56CF: never derived from the name
    QString profileId;
    QString level;
    QString usernameAtIssue;  // the name as it was on the day; renaming later does not change it
    QString curriculumRevision;
    QString assessmentReference;
    QDate issuedOn;

    bool operator==(const CertificateAward &) const = default;
};

// The learner's name as it goes on a certificate: Unicode NFC, without surrounding spaces. Empty when it
// cannot be printed (empty, too long, control or invisible format characters), with the reason in `error`.
QString certificateName(const QString &name, QString *error = nullptr);

// The template with the learner's name, the date (DD/MM/YYYY) and the ID written into its text elements
// `learner-name`, `issue-date` and `certificate-id`, each of which must appear once. The XML is rewritten
// node by node, so a name with <, & or quotes stays text. Empty on failure, with the reason in `error`.
QByteArray personalizeCertificate(const QByteArray &templateSvg, const QString &name, QDate issuedOn,
                                  const QString &certificateId, QString *error = nullptr);

// A new certificate ID for `level` issued in `year`, with 48 random bits: LQ-B1-2026-0E89AB2F56CF.
QString newCertificateId(const QString &level, int year);

// The stable, opaque ID of this database's learner, created the first time it is asked for.
QString profileId(const QSqlDatabase &db);

// The learner's awards, oldest first.
QList<CertificateAward> loadCertificates(const QSqlDatabase &db, const QString &profileId);

struct IssueResult {
    std::optional<CertificateAward> award;
    bool created = false;  // false when the award already existed and is returned unchanged
    QString error;         // why nothing was issued, in words for the learner
};

// Issues `level` to the learner, once per course revision. An existing award comes back unchanged, even if
// the name changed since. Nothing is written unless `evidence` is for this level and eligible.
IssueResult issueCertificate(const QSqlDatabase &db, const QString &profileId, const QString &name,
                             const QString &level, const ReadinessEvidence &evidence, QDate today);

} // namespace lq
