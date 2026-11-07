#include "certificatecontroller.h"
#include "profilecontroller.h"

#include "lq/certificates.h"
#include "lq/database.h"
#include "lq/profile.h"

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTimeZone>
#include <QVariantList>
#include <QVariantMap>

namespace {

const QDate kToday(2026, 11, 6);

lq::Profile profileNamed(const QString &name)
{
    lq::Profile profile;
    profile.name = name;
    profile.reason = QStringLiteral("I want to talk with my partner's family in Sevilla.");
    profile.level = QStringLiteral("A1");
    profile.planDays = 30;
    profile.planStart = QDate(2026, 11, 2);
    profile.createdAt = QDateTime(QDate(2026, 11, 2), QTime(9, 0), QTimeZone::UTC);
    return profile;
}

QVariantMap levelEntry(const CertificateController &controller, const QString &level)
{
    for (const QVariant &entry : controller.levels()) {
        if (entry.toMap().value(QStringLiteral("level")).toString() == level)
            return entry.toMap();
    }
    FAIL("no entry for " << level.toStdString());
    return {};
}

int awardRows(const QSqlDatabase &db)
{
    QSqlQuery q(db);
    REQUIRE(q.exec(QStringLiteral("SELECT COUNT(*) FROM certificate_award")));
    REQUIRE(q.next());
    return q.value(0).toInt();
}

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

// Every question of an attempt right, as the Assessment screen hands it over.
QVariantMap fullMarks()
{
    return {
        {QStringLiteral("reading"), QVariantList{lq::kAssessmentPassageQuestions, lq::kAssessmentPassageQuestions}},
        {QStringLiteral("listening"), QVariantList{lq::kAssessmentPassageQuestions, lq::kAssessmentPassageQuestions}},
        {QStringLiteral("language"), QVariantList{lq::kAssessmentLanguageItems, lq::kAssessmentLanguageItems}},
    };
}

// Evidence handed straight to issueCertificate, for tests of what happens after issuing.
lq::ReadinessEvidence passed(const QString &level)
{
    lq::ReadinessEvidence evidence;
    evidence.level = level;
    evidence.curriculumComplete = true;
    evidence.assessmentPassed = true;
    evidence.skillsVerified = true;
    evidence.assessmentReference = QStringLiteral("test-assessment");
    return evidence;
}

struct Fixture {
    QTemporaryDir dir;
    lq::Database db;
    ProfileController profile{kToday};
    CertificateController certificates{nullptr};

    explicit Fixture(const QString &name)
    {
        REQUIRE(dir.isValid());
        // LQ_KEEP_TEST_FILES=1 keeps the exported SVG and PDF, to look at them.
        if (qEnvironmentVariableIsSet("LQ_KEEP_TEST_FILES")) {
            dir.setAutoRemove(false);
            WARN("Test files kept in " << dir.path().toStdString());
        }
        REQUIRE(db.open(dir.filePath(QStringLiteral("learnquick.sqlite3"))));
        REQUIRE(lq::saveProfile(db.connection(), profileNamed(name)));
        profile.attach(&db);
        certificates.setTemplateDir(QStringLiteral(LQ_ASSETS_DIR "/certificates"));
        certificates.setProfile(&profile);
        certificates.setToday(kToday);
        certificates.attach(&db, dir.path());
    }
};

} // namespace

TEST_CASE("the sample learner sees locked certificates and can never be issued one", "[certificates]")
{
    ProfileController profile(kToday);
    profile.preview(ProfileController::demoProfile(kToday));
    CertificateController certificates(nullptr);
    certificates.setProfile(&profile);
    CHECK_FALSE(certificates.stored());
    CHECK(certificates.learnerName() == QStringLiteral("Cory"));
    for (const QString &level : lq::kCertificateLevels) {
        const QVariantMap entry = levelEntry(certificates, level);
        CHECK_FALSE(entry.value(QStringLiteral("earned")).toBool());
        CHECK_FALSE(entry.value(QStringLiteral("lockedReason")).toString().isEmpty());
        CHECK_FALSE(certificates.issue(level).isEmpty());
    }
    CHECK(certificates.awards().isEmpty());
}

TEST_CASE("the certificate screen reads the welcome name and issues nothing before the lessons and the assessment", "[certificates]")
{
    Fixture f(QStringLiteral("José Ñúñez de la Peña"));
    CHECK(f.certificates.stored());
    CHECK(f.certificates.issuingEnabled());
    CHECK(f.certificates.learnerName() == QStringLiteral("José Ñúñez de la Peña"));
    for (const QString &level : lq::kCertificateLevels) {
        const QString why = f.certificates.issue(level);
        CHECK(why == lq::readinessEvidence(f.db.connection(), lq::profileId(f.db.connection()), level).reason);
        const QVariantMap entry = levelEntry(f.certificates, level);
        CHECK_FALSE(entry.value(QStringLiteral("earned")).toBool());
        CHECK_FALSE(entry.value(QStringLiteral("assessmentOpen")).toBool());
        CHECK_FALSE(entry.value(QStringLiteral("ready")).toBool());
        CHECK(entry.value(QStringLiteral("lessonsDone")).toInt() == 0);
        CHECK(entry.value(QStringLiteral("lessonsNeeded")).toInt() == lq::assessmentUnits(level.toStdString()));
        CHECK(entry.value(QStringLiteral("lockedReason")).toString().contains(QStringLiteral("0 of")));
        // Not open: an attempt handed over anyway is neither stored nor issued.
        const QVariantMap result = f.certificates.finishAssessment(level, fullMarks());
        CHECK(result.value(QStringLiteral("pass")).toBool());
        CHECK_FALSE(result.value(QStringLiteral("saved")).toBool());
        CHECK_FALSE(result.value(QStringLiteral("issued")).toBool());
        CHECK_FALSE(result.value(QStringLiteral("error")).toString().isEmpty());
    }
    CHECK(awardRows(f.db.connection()) == 0);
    CHECK_FALSE(QDir(f.dir.filePath(QStringLiteral("certificates"))).exists());
    CHECK_FALSE(f.certificates.saveSvg(QStringLiteral("A1"), QUrl::fromLocalFile(f.dir.filePath(QStringLiteral("a1.svg")))).isEmpty());
    CHECK_FALSE(QFile::exists(f.dir.filePath(QStringLiteral("a1.svg"))));
}

TEST_CASE("passing a level's assessment issues its certificate; failing it waits for tomorrow", "[certificates]")
{
    Fixture f(QStringLiteral("Ana"));
    finishLessons(f.db.connection(), 10);
    CHECK(levelEntry(f.certificates, QStringLiteral("A1")).value(QStringLiteral("assessmentOpen")).toBool());
    CHECK(levelEntry(f.certificates, QStringLiteral("A2")).value(QStringLiteral("assessmentOpen")).toBool());
    CHECK_FALSE(levelEntry(f.certificates, QStringLiteral("B1")).value(QStringLiteral("assessmentOpen")).toBool());
    // The same draw all day, another the next.
    const QString seed = f.certificates.assessmentSeed(QStringLiteral("A1"));
    CHECK(seed == f.certificates.assessmentSeed(QStringLiteral("A1")));
    CHECK(seed != f.certificates.assessmentSeed(QStringLiteral("A2")));

    // A2 failed: 2 of 5 in listening. Stored, nothing issued, closed until tomorrow.
    QVariantMap scores = fullMarks();
    scores[QStringLiteral("listening")] = QVariantList{2, lq::kAssessmentPassageQuestions};
    QVariantMap result = f.certificates.finishAssessment(QStringLiteral("A2"), scores);
    CHECK(result.value(QStringLiteral("saved")).toBool());
    CHECK_FALSE(result.value(QStringLiteral("pass")).toBool());
    CHECK(result.value(QStringLiteral("overallPassed")).toBool());
    CHECK_FALSE(result.value(QStringLiteral("partsPassed")).toBool());
    CHECK(result.value(QStringLiteral("listening")).toInt() == 40);
    CHECK_FALSE(result.value(QStringLiteral("issued")).toBool());
    QVariantMap a2 = levelEntry(f.certificates, QStringLiteral("A2"));
    CHECK_FALSE(a2.value(QStringLiteral("earned")).toBool());
    CHECK_FALSE(a2.value(QStringLiteral("assessmentOpen")).toBool());
    CHECK(a2.value(QStringLiteral("attempts")).toInt() == 1);
    CHECK(a2.value(QStringLiteral("lockedReason")).toString().contains(QStringLiteral("tomorrow")));
    CHECK_FALSE(f.certificates.finishAssessment(QStringLiteral("A2"), fullMarks()).value(QStringLiteral("saved")).toBool());

    // A1 passed: stored and issued at once, to the welcome name, on the attempt just taken.
    int issued = 0;
    QObject::connect(&f.certificates, &CertificateController::issued, [&issued](const QString &) { ++issued; });
    result = f.certificates.finishAssessment(QStringLiteral("A1"), fullMarks());
    CHECK(result.value(QStringLiteral("saved")).toBool());
    CHECK(result.value(QStringLiteral("pass")).toBool());
    CHECK(result.value(QStringLiteral("issued")).toBool());
    CHECK(result.value(QStringLiteral("error")).toString().isEmpty());
    CHECK(result.value(QStringLiteral("overall")).toInt() == 100);
    CHECK(issued == 1);
    REQUIRE(f.certificates.awards().size() == 1);
    const lq::CertificateAward &award = f.certificates.awards().front();
    CHECK(award.level == QStringLiteral("A1"));
    CHECK(award.usernameAtIssue == QStringLiteral("Ana"));
    const auto attempts = lq::loadAssessments(f.db.connection(), lq::profileId(f.db.connection()), QStringLiteral("A1"));
    REQUIRE(attempts.size() == 1);
    CHECK(award.assessmentReference == attempts.front().reference);
    const QVariantMap a1 = levelEntry(f.certificates, QStringLiteral("A1"));
    CHECK(a1.value(QStringLiteral("earned")).toBool());

    // The next day A2 opens again.
    f.certificates.setToday(kToday.addDays(1));
    CHECK(levelEntry(f.certificates, QStringLiteral("A2")).value(QStringLiteral("assessmentOpen")).toBool());
}

TEST_CASE("an earned certificate opens, saves as SVG and exports as PDF, with the name it was issued to", "[certificates]")
{
    Fixture f(QStringLiteral("José Ñúñez"));
    const QString profileId = lq::profileId(f.db.connection());
    const auto issued = lq::issueCertificate(f.db.connection(), profileId, f.profile.name(), QStringLiteral("A2"),
                                             passed(QStringLiteral("A2")), kToday);
    REQUIRE(issued.award);
    f.certificates.attach(&f.db, f.dir.path());  // as after a restart

    // The learner renames themselves later: the award keeps the name it was issued to.
    REQUIRE(lq::saveProfile(f.db.connection(), profileNamed(QStringLiteral("Pepe"))));
    f.profile.reload();
    CHECK(f.certificates.learnerName() == QStringLiteral("Pepe"));

    const QVariantMap entry = levelEntry(f.certificates, QStringLiteral("A2"));
    CHECK(entry.value(QStringLiteral("earned")).toBool());
    CHECK(entry.value(QStringLiteral("username")).toString() == QStringLiteral("José Ñúñez"));
    CHECK(entry.value(QStringLiteral("issuedOn")).toString() == QStringLiteral("06/11/2026"));
    CHECK(entry.value(QStringLiteral("certificateId")).toString() == issued.award->certificateId);
    CHECK_FALSE(levelEntry(f.certificates, QStringLiteral("A1")).value(QStringLiteral("earned")).toBool());
    CHECK_FALSE(levelEntry(f.certificates, QStringLiteral("B1")).value(QStringLiteral("earned")).toBool());

    // The personalised copy lives in the learner's data folder, next to the one the app draws.
    const QString saved = QDir(f.dir.path()).filePath(QStringLiteral("certificates/%1.svg").arg(issued.award->certificateId));
    const QUrl file = entry.value(QStringLiteral("file")).toUrl();
    REQUIRE(file.isLocalFile());
    CHECK(file.toLocalFile() == QDir(f.dir.path()).filePath(QStringLiteral("certificates/%1.preview.svg").arg(issued.award->certificateId)));
    QFile cached(saved);
    REQUIRE(cached.open(QIODevice::ReadOnly));
    const QByteArray svg = cached.readAll();
    cached.close();
    CHECK(svg.contains(QStringLiteral("José Ñúñez").toUtf8()));
    CHECK_FALSE(svg.contains("Pepe"));
    CHECK(svg.contains("font-family=\"Georgia, Times New Roman, serif\""));  // as the template has it, for browsers
    QFile preview(file.toLocalFile());
    REQUIRE(preview.open(QIODevice::ReadOnly));
    const QByteArray drawn = preview.readAll();
    preview.close();  // Windows deletes no open file, and the test removes it below
    CHECK(drawn.contains(QStringLiteral("José Ñúñez").toUtf8()));
    if (QFontDatabase::hasFamily(QStringLiteral("Georgia"))) {
        CHECK(drawn.contains("font-family=\"Georgia\""));
        CHECK_FALSE(drawn.contains("Georgia, Times New Roman"));
    }

    // Asking again issues nothing new: the award there is the answer.
    CHECK(f.certificates.issue(QStringLiteral("A2")).isEmpty());
    CHECK(awardRows(f.db.connection()) == 1);

    const QString svgCopy = f.dir.filePath(QStringLiteral("export/my certificate.svg"));
    REQUIRE(QDir().mkpath(f.dir.filePath(QStringLiteral("export"))));
    CHECK(f.certificates.saveSvg(QStringLiteral("A2"), QUrl::fromLocalFile(svgCopy)).isEmpty());
    QFile copy(svgCopy);
    REQUIRE(copy.open(QIODevice::ReadOnly));
    CHECK(copy.readAll() == svg);

    const QString pdfPath = f.dir.filePath(QStringLiteral("export/my certificate.pdf"));
    CHECK(f.certificates.exportPdf(QStringLiteral("A2"), QUrl::fromLocalFile(pdfPath)).isEmpty());
    QFile pdf(pdfPath);
    REQUIRE(pdf.open(QIODevice::ReadOnly));
    const QByteArray pdfBytes = pdf.readAll();
    CHECK(pdfBytes.startsWith("%PDF"));
    CHECK(pdfBytes.size() > 10'000);
    pdf.close();

    // The PDF is written beside the target and renamed at the end: it replaces the old file whole and leaves no
    // temporary file, and an export that cannot be written leaves nothing behind.
    REQUIRE(pdf.open(QIODevice::WriteOnly | QIODevice::Truncate));
    REQUIRE(pdf.write("an older file") > 0);
    pdf.close();
    CHECK(f.certificates.exportPdf(QStringLiteral("A2"), QUrl::fromLocalFile(pdfPath)).isEmpty());
    REQUIRE(pdf.open(QIODevice::ReadOnly));
    CHECK(pdf.readAll().startsWith("%PDF"));
    pdf.close();
    CHECK(QDir(f.dir.filePath(QStringLiteral("export"))).entryList(QDir::Files).size() == 2);
    const QString nowhere = f.dir.filePath(QStringLiteral("missing folder/certificate.pdf"));
    CHECK_FALSE(f.certificates.exportPdf(QStringLiteral("A2"), QUrl::fromLocalFile(nowhere)).isEmpty());
    CHECK_FALSE(QFile::exists(nowhere));

    // A deleted copy comes back from the stored record, the same as before.
    REQUIRE(QFile::remove(saved));
    REQUIRE(QFile::remove(file.toLocalFile()));
    const QVariantMap again = levelEntry(f.certificates, QStringLiteral("A2"));
    CHECK(QFile::exists(again.value(QStringLiteral("file")).toUrl().toLocalFile()));
    QFile rebuilt(saved);
    REQUIRE(rebuilt.open(QIODevice::ReadOnly));
    CHECK(rebuilt.readAll() == svg);

    CHECK(f.certificates.suggestedFile(QStringLiteral("A2"), QStringLiteral("pdf")).fileName()
          == QStringLiteral("LearnQuick-A2-certificate-%1.pdf").arg(issued.award->certificateId));
}

int main(int argc, char *argv[])
{
    // PDF export draws text, which needs a GUI application (offscreen in CI).
    QGuiApplication app(argc, argv);
    return Catch::Session().run(argc, argv);
}
