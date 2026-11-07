#include "lq/certificates.h"
#include "lq/database.h"
#include "lq/learner.h"
#include "lq/profile.h"

#include <catch2/catch_test_macros.hpp>

#include <QFile>
#include <QRegularExpression>
#include <QSet>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTimeZone>
#include <QXmlStreamReader>

namespace {

QByteArray templateFor(const QString &level)
{
    QFile file(QStringLiteral(LQ_ASSETS_DIR "/certificates/certificate_%1.svg").arg(level));
    REQUIRE(file.open(QIODevice::ReadOnly));
    return file.readAll();
}

// The text of the element with this id, including its <tspan>s; empty when there is none.
QString textOf(const QByteArray &svg, const QString &id)
{
    QXmlStreamReader reader(svg);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement() && reader.attributes().value(QStringLiteral("id")) == id)
            return reader.readElementText(QXmlStreamReader::IncludeChildElements);
    }
    return {};
}

// Every piece of text in the document, element by element, with <tspan>s joined to their line.
QStringList textLines(const QByteArray &svg)
{
    QStringList lines;
    QXmlStreamReader reader(svg);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement() && reader.name() == QStringLiteral("text"))
            lines << reader.readElementText(QXmlStreamReader::IncludeChildElements);
    }
    REQUIRE_FALSE(reader.hasError());
    return lines;
}

QString attributeOf(const QByteArray &svg, const QString &id, const QString &attribute)
{
    QXmlStreamReader reader(svg);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement() && reader.attributes().value(QStringLiteral("id")) == id)
            return reader.attributes().value(attribute).toString();
    }
    return {};
}

lq::ReadinessEvidence passed(const QString &level)
{
    lq::ReadinessEvidence evidence;
    evidence.level = level;
    evidence.curriculumComplete = true;
    evidence.assessmentPassed = true;
    evidence.skillsVerified = true;
    evidence.assessmentReference = QStringLiteral("test-assessment-%1").arg(level);
    return evidence;
}

int awardRows(const QSqlDatabase &db)
{
    QSqlQuery q(db);
    REQUIRE(q.exec(QStringLiteral("SELECT COUNT(*) FROM certificate_award")));
    REQUIRE(q.next());
    return q.value(0).toInt();
}

const QString kId = QStringLiteral("LQ-A2-2026-0E89AB2F56CF");

// The first `count` lessons done, as the lesson screen records them.
void finishLessons(const QSqlDatabase &db, int count)
{
    QSqlQuery q(db);
    for (int unit = 0; unit < count; ++unit) {
        q.prepare(QStringLiteral("INSERT OR REPLACE INTO unit_result (unit, first_on, score, best, attempts) "
                                 "VALUES (?, '2026-11-02', 0.9, 0.9, 1)"));
        q.addBindValue(unit);
        REQUIRE(q.exec());
    }
}

// An attempt with every part asked in full: right answers in reading, listening and language.
lq::AssessmentScores scores(int reading, int listening, int language)
{
    return {{reading, lq::kAssessmentPassageQuestions},
            {listening, lq::kAssessmentPassageQuestions},
            {language, lq::kAssessmentLanguageItems}};
}

} // namespace

TEST_CASE("certificate names keep the learner's spelling, accents and inner spaces", "[certificates]")
{
    CHECK(lq::certificateName(QStringLiteral("  José Ñúñez de la Peña  ")) == QStringLiteral("José Ñúñez de la Peña"));
    // A decomposed accent (e + combining acute) prints as the same "é".
    CHECK(lq::certificateName(QStringLiteral("José")) == QStringLiteral("José"));
    CHECK(lq::certificateName(QStringLiteral("Zoë 🌮")) == QStringLiteral("Zoë 🌮"));
    QString error;
    CHECK(lq::certificateName(QStringLiteral("   "), &error).isEmpty());
    CHECK_FALSE(error.isEmpty());
    CHECK(lq::certificateName(QString(61, u'a')).isEmpty());
    CHECK(lq::certificateName(QString(60, u'a')).size() == 60);
    CHECK(lq::certificateName(QStringLiteral("Ana\nBo")).isEmpty());
    CHECK(lq::certificateName(QStringLiteral("Ana​Bo")).isEmpty());  // zero-width space
    CHECK(lq::certificateName(QStringLiteral("Ana‮Bo")).isEmpty());  // right-to-left override
    CHECK(lq::certificateName(QString(QChar(0xD800)) + QStringLiteral("Ana")).isEmpty());  // lone surrogate
}

TEST_CASE("each template gets the name, date and ID, and keeps its callout and disclaimer", "[certificates]")
{
    for (const QString &level : lq::kCertificateLevels) {
        INFO(level.toStdString());
        const QString id = QStringLiteral("LQ-%1-2026-0123456789AB").arg(level);
        const QByteArray svg = lq::personalizeCertificate(templateFor(level), QStringLiteral("José Ñúñez"),
                                                          QDate(2026, 11, 6), id);
        REQUIRE_FALSE(svg.isEmpty());
        CHECK(textOf(svg, QStringLiteral("learner-name")) == QStringLiteral("José Ñúñez"));
        CHECK(textOf(svg, QStringLiteral("issue-date")) == QStringLiteral("06/11/2026"));
        CHECK(textOf(svg, QStringLiteral("certificate-id")) == id);
        CHECK(textOf(svg, QStringLiteral("level-medal")) == level);
        const QStringList lines = textLines(svg);
        CHECK(lines.contains(QStringLiteral("Follow FredPlusPlus10 on GitHub for more.")));
        CHECK(lines.contains(QStringLiteral("Congratulations from LearnQuick and its developer.")));
        CHECK(lines.contains(QStringLiteral("Spanish Level %1").arg(level)));
        CHECK(lines.join(u'\n').contains(QStringLiteral("Not an official CEFR qualification")));
        CHECK_FALSE(svg.contains("[LEARNER NAME]"));
        CHECK_FALSE(svg.toLower().contains("signature"));
        // The namespaces come through as written: no generated prefixes such as <n1:svg>.
        CHECK(svg.contains("<svg xmlns=\"http://www.w3.org/2000/svg\""));
        CHECK_FALSE(svg.contains("n1:"));
        CHECK(svg.contains("xlink:href"));
    }
}

TEST_CASE("a name with XML characters stays text in the certificate", "[certificates]")
{
    const QString name = QStringLiteral("Ana <tspan>\"Bo\"</tspan> & 'Cy'");
    const QByteArray svg = lq::personalizeCertificate(templateFor(QStringLiteral("A1")), name, QDate(2026, 11, 6),
                                                      QStringLiteral("LQ-A1-2026-0123456789AB"));
    REQUIRE_FALSE(svg.isEmpty());
    CHECK(textOf(svg, QStringLiteral("learner-name")) == name);
    CHECK_FALSE(svg.contains("<tspan>\"Bo\""));
    CHECK(svg.contains("&lt;tspan&gt;"));
    CHECK(svg.contains("&amp; 'Cy'"));
}

TEST_CASE("long names get a smaller type size instead of being cut", "[certificates]")
{
    const QByteArray tmpl = templateFor(QStringLiteral("B1"));
    const QString id = QStringLiteral("LQ-B1-2026-0123456789AB");
    const QByteArray shortName = lq::personalizeCertificate(tmpl, QStringLiteral("Ana"), QDate(2026, 11, 6), id);
    CHECK(attributeOf(shortName, QStringLiteral("learner-name"), QStringLiteral("font-size")) == QStringLiteral("43"));
    CHECK(attributeOf(shortName, QStringLiteral("learner-name"), QStringLiteral("textLength")).isEmpty());

    const QString longName = QStringLiteral("María de los Ángeles Fernández-Villaverde Ruiz");  // 47 characters
    const QByteArray longSvg = lq::personalizeCertificate(tmpl, longName, QDate(2026, 11, 6), id);
    CHECK(textOf(longSvg, QStringLiteral("learner-name")) == longName);
    CHECK(attributeOf(longSvg, QStringLiteral("learner-name"), QStringLiteral("font-size")).toInt() < 43);
    CHECK(attributeOf(longSvg, QStringLiteral("learner-name"), QStringLiteral("textLength")) == QStringLiteral("600"));
}

TEST_CASE("personalizing refuses bad IDs, dates and templates", "[certificates]")
{
    const QByteArray tmpl = templateFor(QStringLiteral("A1"));
    QString error;
    CHECK(lq::personalizeCertificate(tmpl, QStringLiteral("Ana"), QDate(2026, 11, 6), QStringLiteral("LQ-A1-DEMO-ONLY"), &error).isEmpty());
    CHECK_FALSE(error.isEmpty());
    CHECK(lq::personalizeCertificate(tmpl, QStringLiteral("Ana"), QDate(), QStringLiteral("LQ-A1-2026-0123456789AB")).isEmpty());
    CHECK(lq::personalizeCertificate(tmpl, QString(), QDate(2026, 11, 6), QStringLiteral("LQ-A1-2026-0123456789AB")).isEmpty());

    QByteArray missing = tmpl;
    missing.replace("id=\"issue-date\"", "id=\"something-else\"");
    CHECK(lq::personalizeCertificate(missing, QStringLiteral("Ana"), QDate(2026, 11, 6), QStringLiteral("LQ-A1-2026-0123456789AB")).isEmpty());

    QByteArray twice = tmpl;
    twice.replace("<text id=\"certificate-id\"", "<text id=\"learner-name\"");
    CHECK(lq::personalizeCertificate(twice, QStringLiteral("Ana"), QDate(2026, 11, 6), QStringLiteral("LQ-A1-2026-0123456789AB")).isEmpty());

    const QByteArray withDtd = "<!DOCTYPE svg [<!ENTITY x \"boom\">]>" + tmpl;
    CHECK(lq::personalizeCertificate(withDtd, QStringLiteral("Ana"), QDate(2026, 11, 6), QStringLiteral("LQ-A1-2026-0123456789AB")).isEmpty());
}

TEST_CASE("certificate IDs have the level, the year and 12 random hex digits", "[certificates]")
{
    static const QRegularExpression pattern(QStringLiteral("^LQ-B1-2026-[0-9A-F]{12}$"));
    QSet<QString> ids;
    for (int i = 0; i < 1000; ++i) {
        const QString id = lq::newCertificateId(QStringLiteral("B1"), 2026);
        CHECK(pattern.match(id).hasMatch());
        ids.insert(id);
    }
    CHECK(ids.size() == 1000);
}

TEST_CASE("the app's evidence needs the level's lessons and a passed assessment", "[certificates]")
{
    STATIC_CHECK(lq::kCertificateIssuanceEnabled);
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    lq::Database db;
    REQUIRE(db.open(dir.filePath(QStringLiteral("learnquick.sqlite3"))));
    const QString profile = lq::profileId(db.connection());
    const QDate today(2026, 11, 6);

    // Nothing done: every level says what to finish first, and nothing is issued.
    for (const QString &level : lq::kCertificateLevels) {
        const lq::ReadinessEvidence evidence = lq::readinessEvidence(db.connection(), profile, level);
        CHECK_FALSE(evidence.eligible());
        CHECK_FALSE(evidence.curriculumComplete);
        CHECK(evidence.reason.contains(level));
        CHECK(evidence.reason.contains(QStringLiteral("0 of")));
        const lq::IssueResult result = lq::issueCertificate(db.connection(), profile, QStringLiteral("Ana"), level,
                                                            evidence, today);
        CHECK_FALSE(result.award.has_value());
        CHECK(result.error == evidence.reason);
        const lq::AssessmentAccess access = lq::assessmentAccess(db.connection(), profile, level, today);
        CHECK_FALSE(access.open);
        CHECK(access.lessons.needed == lq::assessmentUnits(level.toStdString()));
    }
    CHECK(awardRows(db.connection()) == 0);

    // Phase 1 done: the A1 assessment opens, A2 and B1 stay closed; a passed A1 attempt makes A1 eligible.
    finishLessons(db.connection(), 5);
    CHECK(lq::assessmentAccess(db.connection(), profile, QStringLiteral("A1"), today).open);
    CHECK_FALSE(lq::assessmentAccess(db.connection(), profile, QStringLiteral("A2"), today).open);
    CHECK_FALSE(lq::readinessEvidence(db.connection(), profile, QStringLiteral("A1")).eligible());
    const auto record = lq::saveAssessment(db.connection(), profile, QStringLiteral("A1"), scores(4, 4, 9), today);
    REQUIRE(record);
    CHECK(record->passed());
    const lq::ReadinessEvidence a1 = lq::readinessEvidence(db.connection(), profile, QStringLiteral("A1"));
    CHECK(a1.eligible());
    CHECK(a1.assessmentReference == record->reference);
    CHECK(a1.reason.isEmpty());
    // A1's attempt says nothing about A2.
    CHECK_FALSE(lq::readinessEvidence(db.connection(), profile, QStringLiteral("A2")).eligible());
    const lq::IssueResult issued = lq::issueCertificate(db.connection(), profile, QStringLiteral("Ana"),
                                                        QStringLiteral("A1"), a1, today);
    REQUIRE(issued.award);
    CHECK(issued.award->assessmentReference == record->reference);
}

TEST_CASE("an assessment is taken once a day, only when finished, and stays as it was stored", "[certificates]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    lq::Database db;
    REQUIRE(db.open(dir.filePath(QStringLiteral("learnquick.sqlite3"))));
    const QString profile = lq::profileId(db.connection());
    const QDate monday(2026, 11, 9);
    finishLessons(db.connection(), 10);

    QString error;
    // An unfinished attempt, or one with made-up totals, is never stored.
    CHECK_FALSE(lq::saveAssessment(db.connection(), profile, QStringLiteral("A2"), {{5, 5}, {5, 5}, {6, 6}}, monday, &error));
    CHECK_FALSE(error.isEmpty());
    CHECK_FALSE(lq::saveAssessment(db.connection(), profile, QStringLiteral("A2"), {{6, 5}, {5, 5}, {12, 12}}, monday));
    CHECK_FALSE(lq::saveAssessment(db.connection(), profile, QStringLiteral("C1"), scores(5, 5, 12), monday));
    CHECK_FALSE(lq::saveAssessment(db.connection(), QString(), QStringLiteral("A2"), scores(5, 5, 12), monday));

    // 2 of 5 in listening fails, whatever the rest: the part is below 60%.
    const auto failed = lq::saveAssessment(db.connection(), profile, QStringLiteral("A2"), scores(5, 2, 12), monday);
    REQUIRE(failed);
    CHECK_FALSE(failed->passed());
    lq::AssessmentAccess access = lq::assessmentAccess(db.connection(), profile, QStringLiteral("A2"), monday);
    CHECK_FALSE(access.open);
    CHECK(access.attempts == 1);
    CHECK(access.bestPercent == 86);  // 19 of 22
    CHECK(access.reason.contains(QStringLiteral("tomorrow")));
    CHECK_FALSE(lq::saveAssessment(db.connection(), profile, QStringLiteral("A2"), scores(5, 5, 12), monday, &error));
    CHECK(error.contains(QStringLiteral("tomorrow")));
    CHECK_FALSE(lq::readinessEvidence(db.connection(), profile, QStringLiteral("A2")).eligible());

    // The next day it opens again; once passed, it closes for good.
    const QDate tuesday = monday.addDays(1);
    CHECK(lq::assessmentAccess(db.connection(), profile, QStringLiteral("A2"), tuesday).open);
    const auto passedAttempt = lq::saveAssessment(db.connection(), profile, QStringLiteral("A2"), scores(3, 3, 10), tuesday);
    REQUIRE(passedAttempt);
    CHECK(passedAttempt->passed());
    access = lq::assessmentAccess(db.connection(), profile, QStringLiteral("A2"), tuesday.addDays(1));
    CHECK_FALSE(access.open);
    CHECK(access.passed);
    CHECK(lq::loadAssessments(db.connection(), profile, QStringLiteral("A2")).size() == 2);
    CHECK(lq::readinessEvidence(db.connection(), profile, QStringLiteral("A2")).assessmentReference
          == passedAttempt->reference);

    // Stored attempts are never changed or removed, not even straight in SQL.
    QSqlQuery q(db.connection());
    CHECK_FALSE(q.exec(QStringLiteral("UPDATE assessment_result SET listening_right = 5")));
    CHECK_FALSE(q.exec(QStringLiteral("DELETE FROM assessment_result")));
    CHECK(lq::loadAssessments(db.connection(), profile, QStringLiteral("A2")).size() == 2);
    CHECK_FALSE(lq::loadAssessments(db.connection(), profile, QStringLiteral("A2")).front().passed());
    // Another learner's attempts are not this one's.
    CHECK(lq::loadAssessments(db.connection(), QStringLiteral("someone-else"), QStringLiteral("A2")).isEmpty());
}

TEST_CASE("evidence counts only when every check passed, for the same level", "[certificates]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    lq::Database db;
    REQUIRE(db.open(dir.filePath(QStringLiteral("learnquick.sqlite3"))));
    const QString profile = lq::profileId(db.connection());
    const QDate today(2026, 11, 6);

    lq::ReadinessEvidence noSkills = passed(QStringLiteral("A1"));
    noSkills.skillsVerified = false;
    lq::ReadinessEvidence noReference = passed(QStringLiteral("A1"));
    noReference.assessmentReference.clear();
    lq::ReadinessEvidence noCurriculum = passed(QStringLiteral("A1"));
    noCurriculum.curriculumComplete = false;
    lq::ReadinessEvidence notPassed = passed(QStringLiteral("A1"));
    notPassed.assessmentPassed = false;
    for (const auto &evidence : {noSkills, noReference, noCurriculum, notPassed})
        CHECK_FALSE(lq::issueCertificate(db.connection(), profile, QStringLiteral("Ana"), QStringLiteral("A1"), evidence, today).award);
    // A2 evidence does not give A1, and A1 evidence does not give B1.
    CHECK_FALSE(lq::issueCertificate(db.connection(), profile, QStringLiteral("Ana"), QStringLiteral("A1"), passed(QStringLiteral("A2")), today).award);
    CHECK_FALSE(lq::issueCertificate(db.connection(), profile, QStringLiteral("Ana"), QStringLiteral("B1"), passed(QStringLiteral("A1")), today).award);
    CHECK_FALSE(lq::issueCertificate(db.connection(), profile, QStringLiteral("Ana"), QStringLiteral("C1"), passed(QStringLiteral("C1")), today).award);
    CHECK_FALSE(lq::issueCertificate(db.connection(), QString(), QStringLiteral("Ana"), QStringLiteral("A1"), passed(QStringLiteral("A1")), today).award);
    CHECK(awardRows(db.connection()) == 0);
}

TEST_CASE("an award is issued once, keeps its name and is never changed", "[certificates]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    lq::CertificateAward first;
    QString profile;
    {
        lq::Database db;
        REQUIRE(db.open(path));
        profile = lq::profileId(db.connection());
        CHECK(lq::profileId(db.connection()) == profile);
        const lq::IssueResult result = lq::issueCertificate(db.connection(), profile, QStringLiteral(" José Ñúñez "),
                                                            QStringLiteral("A2"), passed(QStringLiteral("A2")), QDate(2026, 11, 6));
        REQUIRE(result.award);
        CHECK(result.created);
        first = *result.award;
        CHECK(first.usernameAtIssue == QStringLiteral("José Ñúñez"));
        CHECK(first.level == QStringLiteral("A2"));
        CHECK(first.curriculumRevision == lq::kCurriculumRevision);
        CHECK(first.assessmentReference == QStringLiteral("test-assessment-A2"));
        CHECK(first.issuedOn == QDate(2026, 11, 6));
        CHECK(first.certificateId.startsWith(QStringLiteral("LQ-A2-2026-")));
        CHECK_FALSE(first.certificateId.contains(QStringLiteral("Jos")));
    }
    // After a restart, and after the learner renamed themselves, the award is the same one.
    lq::Database db;
    REQUIRE(db.open(path));
    CHECK(lq::profileId(db.connection()) == profile);
    const lq::IssueResult again = lq::issueCertificate(db.connection(), profile, QStringLiteral("Pepe"), QStringLiteral("A2"),
                                                       passed(QStringLiteral("A2")), QDate(2027, 1, 1));
    REQUIRE(again.award);
    CHECK_FALSE(again.created);
    CHECK(*again.award == first);
    CHECK(lq::loadCertificates(db.connection(), profile) == QList<lq::CertificateAward>{first});

    // The database refuses to change or remove it.
    QSqlQuery q(db.connection());
    CHECK_FALSE(q.exec(QStringLiteral("UPDATE certificate_award SET username_at_issue = 'Pepe'")));
    CHECK_FALSE(q.exec(QStringLiteral("DELETE FROM certificate_award")));
    CHECK(lq::loadCertificates(db.connection(), profile) == QList<lq::CertificateAward>{first});

    // A later level is a separate award with its own ID.
    const lq::IssueResult b1 = lq::issueCertificate(db.connection(), profile, QStringLiteral("Pepe"), QStringLiteral("B1"),
                                                    passed(QStringLiteral("B1")), QDate(2027, 1, 1));
    REQUIRE(b1.award);
    CHECK(b1.award->certificateId != first.certificateId);
    CHECK(b1.award->usernameAtIssue == QStringLiteral("Pepe"));
    CHECK(lq::loadCertificates(db.connection(), profile).size() == 2);
}

TEST_CASE("two learners never see each other's certificates", "[certificates]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    lq::Database db;
    REQUIRE(db.open(dir.filePath(QStringLiteral("learnquick.sqlite3"))));
    const QDate today(2026, 11, 6);
    const auto ana = lq::issueCertificate(db.connection(), QStringLiteral("profile-ana"), QStringLiteral("Ana"),
                                          QStringLiteral("A1"), passed(QStringLiteral("A1")), today);
    const auto bo = lq::issueCertificate(db.connection(), QStringLiteral("profile-bo"), QStringLiteral("Bo"),
                                         QStringLiteral("A1"), passed(QStringLiteral("A1")), today);
    REQUIRE(ana.award);
    REQUIRE(bo.award);
    CHECK(ana.award->certificateId != bo.award->certificateId);
    const auto anas = lq::loadCertificates(db.connection(), QStringLiteral("profile-ana"));
    REQUIRE(anas.size() == 1);
    CHECK(anas.front().usernameAtIssue == QStringLiteral("Ana"));
    const auto bos = lq::loadCertificates(db.connection(), QStringLiteral("profile-bo"));
    REQUIRE(bos.size() == 1);
    CHECK(bos.front().usernameAtIssue == QStringLiteral("Bo"));
    CHECK(lq::loadCertificates(db.connection(), QStringLiteral("profile-cy")).isEmpty());
}

TEST_CASE("restoring a backup leaves certificates and the profile ID alone", "[certificates]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    lq::Database db;
    REQUIRE(db.open(dir.filePath(QStringLiteral("learnquick.sqlite3"))));
    lq::Profile profile;
    profile.name = QStringLiteral("Ana");
    profile.reason = QStringLiteral("I want to talk with my neighbours in Spanish every day.");
    profile.level = QStringLiteral("A2");
    profile.planStart = QDate(2026, 11, 2);
    profile.createdAt = QDateTime(QDate(2026, 11, 2), QTime(9, 0), QTimeZone::UTC);
    REQUIRE(lq::saveProfile(db.connection(), profile));
    const QString id = lq::profileId(db.connection());
    const auto award = lq::issueCertificate(db.connection(), id, profile.name, QStringLiteral("A1"),
                                            passed(QStringLiteral("A1")), QDate(2026, 11, 6));
    REQUIRE(award.award);

    lq::Profile restored = profile;
    restored.name = QStringLiteral("Bo");
    const lq::LearnerState state = lq::defaultState(lq::Date(std::chrono::year(2026) / 11 / 2));
    REQUIRE(lq::restoreLearner(db.connection(), restored, state));
    CHECK(lq::profileId(db.connection()) == id);
    CHECK(lq::loadCertificates(db.connection(), id) == QList<lq::CertificateAward>{*award.award});
}

TEST_CASE("restoring a backup leaves assessment attempts alone", "[certificates]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    lq::Database db;
    REQUIRE(db.open(dir.filePath(QStringLiteral("learnquick.sqlite3"))));
    lq::Profile profile;
    profile.name = QStringLiteral("Ana");
    profile.reason = QStringLiteral("I want to talk with my neighbours in Spanish every day.");
    profile.level = QStringLiteral("A1");
    profile.planStart = QDate(2026, 11, 2);
    profile.createdAt = QDateTime(QDate(2026, 11, 2), QTime(9, 0), QTimeZone::UTC);
    REQUIRE(lq::saveProfile(db.connection(), profile));
    const QString id = lq::profileId(db.connection());
    finishLessons(db.connection(), 5);
    REQUIRE(lq::saveAssessment(db.connection(), id, QStringLiteral("A1"), scores(2, 5, 12), QDate(2026, 11, 6)));

    const lq::LearnerState state = lq::defaultState(lq::Date(std::chrono::year(2026) / 11 / 2));
    REQUIRE(lq::restoreLearner(db.connection(), profile, state));
    CHECK(lq::loadAssessments(db.connection(), id, QStringLiteral("A1")).size() == 1);
}

TEST_CASE("a 0.4.5 database gains the certificate table and keeps everything else", "[certificates]")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("learnquick.sqlite3"));
    lq::Profile profile;
    profile.name = QStringLiteral("José");
    profile.reason = QStringLiteral("I want to talk with my neighbours in Spanish every day.");
    profile.level = QStringLiteral("A1");
    profile.planStart = QDate(2026, 11, 2);
    profile.createdAt = QDateTime(QDate(2026, 11, 2), QTime(9, 0), QTimeZone::UTC);
    {
        lq::Database db;
        REQUIRE(db.open(path));
        REQUIRE(lq::saveProfile(db.connection(), profile));
        QSqlQuery q(db.connection());
        REQUIRE(q.exec(QStringLiteral("INSERT INTO setting (key, value) VALUES ('theme', 'dark')")));
        REQUIRE(q.exec(QStringLiteral("DROP TABLE assessment_result")));  // schema 10
        REQUIRE(q.exec(QStringLiteral("DROP TABLE certificate_award")));
        REQUIRE(q.exec(QStringLiteral("ALTER TABLE input_log DROP COLUMN imported")));  // schema 9
        REQUIRE(q.exec(QStringLiteral("PRAGMA user_version = 7")));
    }
    lq::Database db;
    REQUIRE(db.open(path));
    CHECK(db.schemaVersion() == lq::Database::latestSchemaVersion());
    CHECK(lq::loadProfile(db.connection()) == profile);
    QSqlQuery q(db.connection());
    REQUIRE(q.exec(QStringLiteral("SELECT value FROM setting WHERE key = 'theme'")));
    REQUIRE(q.next());
    CHECK(q.value(0).toString() == QStringLiteral("dark"));
    CHECK(lq::loadCertificates(db.connection(), lq::profileId(db.connection())).isEmpty());
}
