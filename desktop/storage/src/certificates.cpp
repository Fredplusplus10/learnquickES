#include "lq/certificates.h"

#include <QHash>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <algorithm>
#include <cmath>

namespace lq {

namespace {

const QString kProfileIdKey = QStringLiteral("profile_id");

bool knownLevel(const QString &level)
{
    return kCertificateLevels.contains(level);
}

// The template's name line fits about 30 characters at its own size; longer names get smaller, never cut.
int nameFontSize(qsizetype length)
{
    const double size = 43.0 * 30.0 / static_cast<double>(std::max<qsizetype>(30, length));
    return std::clamp(static_cast<int>(std::lround(size)), 21, 43);
}

CertificateAward awardFrom(const QSqlQuery &q)
{
    CertificateAward award;
    award.certificateId = q.value(0).toString();
    award.profileId = q.value(1).toString();
    award.level = q.value(2).toString();
    award.usernameAtIssue = q.value(3).toString();
    award.curriculumRevision = q.value(4).toString();
    award.assessmentReference = q.value(5).toString();
    award.issuedOn = QDate::fromString(q.value(6).toString(), Qt::ISODate);
    return award;
}

const char *kAwardColumns = "certificate_id, profile_id, level, username_at_issue, curriculum_revision, "
                            "assessment_reference, issued_on";

std::optional<CertificateAward> findAward(const QSqlDatabase &db, const QString &profileId, const QString &level,
                                          const QString &revision)
{
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT %1 FROM certificate_award WHERE profile_id = ? AND level = ? AND curriculum_revision = ?")
                  .arg(QLatin1StringView(kAwardColumns)));
    q.addBindValue(profileId);
    q.addBindValue(level);
    q.addBindValue(revision);
    if (q.exec() && q.next())
        return awardFrom(q);
    return std::nullopt;
}

} // namespace

QList<AssessmentRecord> loadAssessments(const QSqlDatabase &db, const QString &profileId, const QString &level)
{
    QList<AssessmentRecord> records;
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT reference, profile_id, level, curriculum_revision, taken_on, reading_right, "
                             "reading_total, listening_right, listening_total, language_right, language_total "
                             "FROM assessment_result WHERE profile_id = ? AND level = ? AND curriculum_revision = ? "
                             "ORDER BY taken_on, rowid"));
    q.addBindValue(profileId);
    q.addBindValue(level);
    q.addBindValue(kCurriculumRevision);
    if (!q.exec())
        return records;
    while (q.next()) {
        AssessmentRecord r;
        r.reference = q.value(0).toString();
        r.profileId = q.value(1).toString();
        r.level = q.value(2).toString();
        r.curriculumRevision = q.value(3).toString();
        r.takenOn = QDate::fromString(q.value(4).toString(), Qt::ISODate);
        r.scores.reading = {q.value(5).toInt(), q.value(6).toInt()};
        r.scores.listening = {q.value(7).toInt(), q.value(8).toInt()};
        r.scores.language = {q.value(9).toInt(), q.value(10).toInt()};
        records.append(r);
    }
    return records;
}

LessonProgress lessonProgress(const QSqlDatabase &db, const QString &level)
{
    LessonProgress progress;
    progress.needed = assessmentUnits(level.toStdString());
    if (progress.needed == 0)
        return progress;
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT COUNT(*) FROM unit_result WHERE unit < ?"));
    q.addBindValue(progress.needed);
    if (q.exec() && q.next())
        progress.done = q.value(0).toInt();
    return progress;
}

namespace {

QString lessonsNeededText(const QString &level)
{
    if (level == QStringLiteral("A1"))
        return QStringLiteral("the five lessons of phase 1");
    if (level == QStringLiteral("A2"))
        return QStringLiteral("the ten lessons of phases 1 and 2");
    return QStringLiteral("all 25 lessons");
}

const QString kPassRule = QStringLiteral("70% overall and at least 60% in reading, listening, and grammar and vocabulary");

} // namespace

AssessmentAccess assessmentAccess(const QSqlDatabase &db, const QString &profileId, const QString &level, QDate today)
{
    AssessmentAccess access;
    if (!knownLevel(level)) {
        access.reason = QStringLiteral("LearnQuick has A1, A2 and B1 assessments only.");
        return access;
    }
    access.lessons = lessonProgress(db, level);
    const QList<AssessmentRecord> attempts = profileId.isEmpty() ? QList<AssessmentRecord>{}
                                                                  : loadAssessments(db, profileId, level);
    access.attempts = static_cast<int>(attempts.size());
    bool takenToday = false;
    for (const AssessmentRecord &r : attempts) {
        access.bestPercent = std::max(access.bestPercent, assessmentResult(r.scores).overall);
        access.passed = access.passed || r.passed();
        takenToday = takenToday || r.takenOn == today;
    }
    if (profileId.isEmpty()) {
        access.reason = QStringLiteral("There is no learner profile to take the assessment.");
    } else if (access.passed) {
        access.reason = QStringLiteral("You passed the %1 assessment.").arg(level);
    } else if (!access.lessons.complete()) {
        access.reason = QStringLiteral("Finish %1 to open the %2 assessment: %3 of %4 done.")
                            .arg(lessonsNeededText(level), level)
                            .arg(access.lessons.done)
                            .arg(access.lessons.needed);
    } else if (!today.isValid()) {
        access.reason = QStringLiteral("The assessment has no valid date.");
    } else if (takenToday) {
        access.reason = QStringLiteral("You took the %1 assessment today (best %2%). You can try again tomorrow.")
                            .arg(level)
                            .arg(access.bestPercent);
    } else {
        access.open = true;
        access.reason = access.attempts == 0
                            ? QStringLiteral("Pass the %1 assessment to earn this certificate: %2.").arg(level, kPassRule)
                            : QStringLiteral("Your best %1 assessment so far: %2%. It passes with %3.")
                                  .arg(level)
                                  .arg(access.bestPercent)
                                  .arg(kPassRule);
    }
    return access;
}

std::optional<AssessmentRecord> saveAssessment(const QSqlDatabase &db, const QString &profileId, const QString &level,
                                               const AssessmentScores &scores, QDate today, QString *error)
{
    const auto fail = [error](const QString &why) -> std::optional<AssessmentRecord> {
        if (error)
            *error = why;
        return std::nullopt;
    };
    const AssessmentAccess access = assessmentAccess(db, profileId, level, today);
    if (!access.open)
        return fail(access.reason);
    if (!assessmentResult(scores).complete)
        return fail(QStringLiteral("The assessment was not finished, so it was not saved."));

    AssessmentRecord record;
    record.reference = QStringLiteral("assessment-") + QUuid::createUuid().toString(QUuid::WithoutBraces);
    record.profileId = profileId;
    record.level = level;
    record.curriculumRevision = kCurriculumRevision;
    record.takenOn = today;
    record.scores = scores;

    QSqlQuery insert(db);
    insert.prepare(QStringLiteral("INSERT INTO assessment_result (reference, profile_id, level, curriculum_revision, "
                                  "taken_on, reading_right, reading_total, listening_right, listening_total, "
                                  "language_right, language_total) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    insert.addBindValue(record.reference);
    insert.addBindValue(record.profileId);
    insert.addBindValue(record.level);
    insert.addBindValue(record.curriculumRevision);
    insert.addBindValue(record.takenOn.toString(Qt::ISODate));
    for (const Tally &t : {scores.reading, scores.listening, scores.language}) {
        insert.addBindValue(t.correct);
        insert.addBindValue(t.total);
    }
    if (!insert.exec())
        return fail(QStringLiteral("The assessment could not be saved: %1").arg(insert.lastError().text()));
    return record;
}

ReadinessEvidence readinessEvidence(const QSqlDatabase &db, const QString &profileId, const QString &level)
{
    ReadinessEvidence evidence;
    evidence.level = level;
    if (!knownLevel(level)) {
        evidence.reason = QStringLiteral("LearnQuick issues A1, A2 and B1 certificates only.");
        return evidence;
    }
    evidence.curriculumComplete = lessonProgress(db, level).complete();
    if (!profileId.isEmpty()) {
        for (const AssessmentRecord &r : loadAssessments(db, profileId, level)) {
            const AssessmentResult result = assessmentResult(r.scores);
            if (result.pass()) {
                evidence.assessmentPassed = result.overallPassed;
                evidence.skillsVerified = result.partsPassed;
                evidence.assessmentReference = r.reference;
                break;
            }
        }
    }
    if (!evidence.curriculumComplete) {
        const LessonProgress lessons = lessonProgress(db, level);
        evidence.reason = QStringLiteral("Finish %1, then pass the %2 assessment: %3 of %4 lessons done.")
                              .arg(lessonsNeededText(level), level)
                              .arg(lessons.done)
                              .arg(lessons.needed);
    } else if (!evidence.eligible()) {
        evidence.reason = QStringLiteral("Pass the %1 assessment to earn this certificate: %2.").arg(level, kPassRule);
    }
    return evidence;
}

QString certificateName(const QString &name, QString *error)
{
    const auto fail = [error](const QString &why) {
        if (error)
            *error = why;
        return QString();
    };
    const QString clean = name.trimmed().normalized(QString::NormalizationForm_C);
    if (clean.isEmpty())
        return fail(QStringLiteral("Your profile has no name to put on a certificate."));
    if (clean.size() > kMaxCertificateNameLength)
        return fail(QStringLiteral("Your name is too long for a certificate (%1 characters at most).").arg(kMaxCertificateNameLength));
    const QString unprintable = QStringLiteral("Your name has a character that cannot be printed on a certificate.");
    // A lone half of a surrogate pair is not text.
    for (qsizetype i = 0; i < clean.size(); ++i) {
        const QChar c = clean.at(i);
        if (c.isHighSurrogate() && (i + 1 >= clean.size() || !clean.at(i + 1).isLowSurrogate()))
            return fail(unprintable);
        if (c.isLowSurrogate() && (i == 0 || !clean.at(i - 1).isHighSurrogate()))
            return fail(unprintable);
    }
    // Control and invisible format characters (zero-width joiners, direction marks), line breaks and
    // unassigned code points would print as nothing or as boxes.
    for (const char32_t codePoint : clean.toUcs4()) {
        switch (QChar::category(codePoint)) {
        case QChar::Other_Control:
        case QChar::Other_Format:
        case QChar::Other_NotAssigned:
        case QChar::Other_PrivateUse:
        case QChar::Separator_Line:
        case QChar::Separator_Paragraph:
            return fail(unprintable);
        default:
            break;
        }
    }
    return clean;
}

QString newCertificateId(const QString &level, int year)
{
    const quint64 bits = QRandomGenerator::system()->generate64() & 0xFFFF'FFFF'FFFFull;  // 48 bits
    return QStringLiteral("LQ-%1-%2-%3").arg(level).arg(year, 4, 10, QChar(u'0')).arg(bits, 12, 16, QChar(u'0')).toUpper();
}

QByteArray personalizeCertificate(const QByteArray &templateSvg, const QString &name, QDate issuedOn,
                                  const QString &certificateId, QString *error)
{
    const auto fail = [error](const QString &why) {
        if (error)
            *error = why;
        return QByteArray();
    };
    QString nameError;
    const QString printed = certificateName(name, &nameError);
    if (printed.isEmpty())
        return fail(nameError);
    if (!issuedOn.isValid())
        return fail(QStringLiteral("The certificate has no valid date."));
    static const QRegularExpression idPattern(QStringLiteral("^LQ-(A1|A2|B1)-\\d{4}-[0-9A-F]{12}$"));
    if (!idPattern.match(certificateId).hasMatch())
        return fail(QStringLiteral("The certificate ID is not one LearnQuick generates."));

    const QHash<QString, QString> fields{
        {QStringLiteral("learner-name"), printed},
        {QStringLiteral("issue-date"), issuedOn.toString(QStringLiteral("dd/MM/yyyy"))},
        {QStringLiteral("certificate-id"), certificateId},
    };
    QHash<QString, int> seen;

    // A plain copy, token by token, with qualified names: the template's namespaces and prefixes come out as
    // they went in. The three fields get new text through the writer, which escapes it.
    QXmlStreamReader reader(templateSvg);
    QByteArray out;
    QXmlStreamWriter writer(&out);
    while (!reader.atEnd()) {
        reader.readNext();
        switch (reader.tokenType()) {
        case QXmlStreamReader::StartDocument:
            writer.writeStartDocument(QStringLiteral("1.0"));
            break;
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
            return fail(QStringLiteral("The certificate template has a DTD or entity, which it never should."));
        case QXmlStreamReader::StartElement: {
            const QString id = reader.attributes().value(QStringLiteral("id")).toString();
            const bool field = fields.contains(id);
            if (field && reader.name() != QStringLiteral("text"))
                return fail(QStringLiteral("The template's %1 is not a text element.").arg(id));
            writer.writeStartElement(reader.qualifiedName().toString());
            for (const QXmlStreamNamespaceDeclaration &ns : reader.namespaceDeclarations()) {
                writer.writeAttribute(ns.prefix().isEmpty() ? QStringLiteral("xmlns")
                                                            : QStringLiteral("xmlns:") + ns.prefix().toString(),
                                      ns.namespaceUri().toString());
            }
            const bool isName = id == QStringLiteral("learner-name");
            for (const QXmlStreamAttribute &attribute : reader.attributes()) {
                const QString attributeName = attribute.qualifiedName().toString();
                if (isName && (attributeName == QStringLiteral("font-size") || attributeName == QStringLiteral("textLength")
                               || attributeName == QStringLiteral("lengthAdjust")))
                    continue;
                writer.writeAttribute(attributeName, attribute.value().toString());
            }
            if (!field)
                break;
            ++seen[id];
            const QString value = fields.value(id);
            if (isName) {
                writer.writeAttribute(QStringLiteral("font-size"), QString::number(nameFontSize(value.size())));
                if (value.size() > 36) {
                    writer.writeAttribute(QStringLiteral("textLength"), QStringLiteral("600"));
                    writer.writeAttribute(QStringLiteral("lengthAdjust"), QStringLiteral("spacingAndGlyphs"));
                }
            }
            writer.writeCharacters(value);
            // The placeholder text, and anything else inside the field, is dropped.
            int depth = 1;
            while (depth > 0 && !reader.atEnd()) {
                reader.readNext();
                if (reader.isStartElement())
                    ++depth;
                else if (reader.isEndElement())
                    --depth;
            }
            writer.writeEndElement();
            break;
        }
        case QXmlStreamReader::EndElement:
            writer.writeEndElement();
            break;
        case QXmlStreamReader::Characters:
            if (reader.isCDATA())
                writer.writeCDATA(reader.text().toString());
            else
                writer.writeCharacters(reader.text().toString());
            break;
        case QXmlStreamReader::Comment:
            writer.writeComment(reader.text().toString());
            break;
        case QXmlStreamReader::ProcessingInstruction:
            writer.writeProcessingInstruction(reader.processingInstructionTarget().toString(),
                                              reader.processingInstructionData().toString());
            break;
        case QXmlStreamReader::EndDocument:
            writer.writeEndDocument();
            break;
        default:
            break;
        }
    }
    if (reader.hasError())
        return fail(QStringLiteral("The certificate template is not valid XML: %1").arg(reader.errorString()));
    for (auto it = fields.cbegin(); it != fields.cend(); ++it) {
        if (seen.value(it.key()) != 1)
            return fail(QStringLiteral("The template must have one %1 text element.").arg(it.key()));
    }
    out.append('\n');
    return out;
}

QString profileId(const QSqlDatabase &db)
{
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT value FROM setting WHERE key = ?"));
    q.addBindValue(kProfileIdKey);
    if (q.exec() && q.next() && !q.value(0).toString().isEmpty())
        return q.value(0).toString();
    // INSERT OR IGNORE, then read back: if two openings race, both end up with the same ID.
    QSqlQuery insert(db);
    insert.prepare(QStringLiteral("INSERT OR IGNORE INTO setting (key, value) VALUES (?, ?)"));
    insert.addBindValue(kProfileIdKey);
    insert.addBindValue(QUuid::createUuid().toString(QUuid::WithoutBraces));
    if (!insert.exec())
        return {};
    q.prepare(QStringLiteral("SELECT value FROM setting WHERE key = ?"));
    q.addBindValue(kProfileIdKey);
    return q.exec() && q.next() ? q.value(0).toString() : QString();
}

QList<CertificateAward> loadCertificates(const QSqlDatabase &db, const QString &profileId)
{
    QList<CertificateAward> awards;
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT %1 FROM certificate_award WHERE profile_id = ? ORDER BY issued_on, rowid")
                  .arg(QLatin1StringView(kAwardColumns)));
    q.addBindValue(profileId);
    if (q.exec()) {
        while (q.next())
            awards.append(awardFrom(q));
    }
    return awards;
}

IssueResult issueCertificate(const QSqlDatabase &db, const QString &profileId, const QString &name,
                             const QString &level, const ReadinessEvidence &evidence, QDate today)
{
    IssueResult result;
    if (!knownLevel(level)) {
        result.error = QStringLiteral("LearnQuick issues A1, A2 and B1 certificates only.");
        return result;
    }
    if (profileId.isEmpty()) {
        result.error = QStringLiteral("There is no learner profile to issue a certificate to.");
        return result;
    }
    // Already earned: the same award, as it was issued.
    if (auto existing = findAward(db, profileId, level, kCurriculumRevision)) {
        result.award = existing;
        return result;
    }
    if (evidence.level != level || !evidence.eligible()) {
        result.error = evidence.reason.isEmpty() ? QStringLiteral("The %1 assessment has not been passed yet.").arg(level)
                                                 : evidence.reason;
        return result;
    }
    QString nameError;
    const QString printed = certificateName(name, &nameError);
    if (printed.isEmpty()) {
        result.error = nameError;
        return result;
    }
    if (!today.isValid()) {
        result.error = QStringLiteral("The certificate has no valid date.");
        return result;
    }

    CertificateAward award;
    award.certificateId = newCertificateId(level, today.year());
    award.profileId = profileId;
    award.level = level;
    award.usernameAtIssue = printed;
    award.curriculumRevision = kCurriculumRevision;
    award.assessmentReference = evidence.assessmentReference;
    award.issuedOn = today;

    QSqlQuery insert(db);
    insert.prepare(QStringLiteral("INSERT INTO certificate_award (%1) VALUES (?, ?, ?, ?, ?, ?, ?)")
                       .arg(QLatin1StringView(kAwardColumns)));
    insert.addBindValue(award.certificateId);
    insert.addBindValue(award.profileId);
    insert.addBindValue(award.level);
    insert.addBindValue(award.usernameAtIssue);
    insert.addBindValue(award.curriculumRevision);
    insert.addBindValue(award.assessmentReference);
    insert.addBindValue(award.issuedOn.toString(Qt::ISODate));
    if (!insert.exec()) {
        // Issued at the same moment elsewhere: that award stands, this one is not written.
        if (auto existing = findAward(db, profileId, level, kCurriculumRevision)) {
            result.award = existing;
            return result;
        }
        result.error = QStringLiteral("The certificate could not be saved: %1").arg(insert.lastError().text());
        return result;
    }
    result.award = award;
    result.created = true;
    return result;
}

} // namespace lq
