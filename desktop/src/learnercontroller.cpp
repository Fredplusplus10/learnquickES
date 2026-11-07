#include "learnercontroller.h"

#include "lq/achievements.h"
#include "lq/demo.h"
#include "lq/exercises.h"
#include "lq/learner.h"
#include "lq/speech.h"
#include "qtdate.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QStringList>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

using namespace Qt::Literals::StringLiterals;

namespace {

QString qs(const std::string &s)
{
    return QString::fromStdString(s);
}

QString qs(std::string_view s)
{
    return QString::fromUtf8(s.data(), static_cast<qsizetype>(s.size()));
}

QVariantList qsList(const std::vector<std::string> &list)
{
    QVariantList out;
    for (const std::string &s : list)
        out.append(qs(s));
    return out;
}

// The five tricks of the method (docs/INVESTIGACION-Y-METODO.md), as the lesson header names them.
QString trickName(int trick)
{
    switch (trick) {
    case 1: return QStringLiteral("80/20 & cognates");
    case 2: return QStringLiteral("No English");
    case 3: return QStringLiteral("Comprehensible input");
    case 4: return QStringLiteral("Speak from day one");
    case 5: return QStringLiteral("Hack the grammar");
    default: return {};
    }
}

} // namespace

QVariantMap LearnerController::wordMap(const lq::Vocab &v)
{
    QString article;
    QString word = qs(v.es);
    if (v.gender) {
        const qsizetype space = word.indexOf(u' ');
        if (space > 0) {
            article = word.left(space);
            word = word.mid(space + 1);
        }
    }
    return {
        {QStringLiteral("id"), qs(v.id)},
        {QStringLiteral("es"), qs(v.es)},
        {QStringLiteral("en"), qs(v.en)},
        {QStringLiteral("example"), qs(v.example)},
        {QStringLiteral("gender"), v.gender ? QString(QChar::fromLatin1(v.gender)) : QString()},
        {QStringLiteral("article"), article},
        {QStringLiteral("word"), word},
    };
}

namespace {

int percent(double ratio)
{
    return static_cast<int>(std::lround(ratio * 100));
}

// Best result in percent, or -1 before the first attempt.
int bestPercent(const std::optional<lq::LearnerState> &state, const std::map<int, lq::Score> lq::LearnerState::*scores, int key)
{
    if (!state)
        return -1;
    const auto &map = (*state).*scores;
    const auto it = map.find(key);
    return it == map.end() ? -1 : percent(it->second.best);
}

QString plural(int n, const QString &one, const QString &many)
{
    return n == 1 ? one : many.arg(n);
}

} // namespace

LearnerController::LearnerController(ProfileController *profile, QDate today, bool demo, const QString &coursePath,
                                     QObject *parent)
    : QObject(parent)
    , m_profile(profile)
    , m_today(today)
    , m_demo(demo)
{
    QFile file(coursePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_courseError = QStringLiteral("Cannot read the course (%1): %2").arg(coursePath, file.errorString());
    } else {
        const QByteArray json = file.readAll();
        std::string error;
        m_course = lq::parseCourse(std::string_view(json.constData(), static_cast<std::size_t>(json.size())), &error);
        if (!m_course)
            m_courseError = QStringLiteral("The course in %1 is damaged: %2").arg(coursePath, qs(error));
    }
    // Beside the course, in the resources or the source tree.
    const QString assessmentsPath = QFileInfo(coursePath).dir().filePath(QStringLiteral("assessments.json"));
    if (QFile assessments(assessmentsPath); assessments.open(QIODevice::ReadOnly)) {
        const QByteArray json = assessments.readAll();
        std::string error;
        m_assessments = lq::parseAssessments(std::string_view(json.constData(), static_cast<std::size_t>(json.size())), &error);
        if (!m_assessments)
            qWarning("The level assessments in %s are damaged: %s", qPrintable(assessmentsPath), error.c_str());
    } else {
        qWarning("Cannot read the level assessments (%s)", qPrintable(assessmentsPath));
    }
    m_error = m_courseError;
    connect(m_profile, &ProfileController::changed, this, &LearnerController::load);
    load();
}

QString LearnerController::bundledCoursePath()
{
    // qt_add_qml_module compiles content/course.json into the module's resources.
    return QStringLiteral(":/qt/qml/LearnQuick/content/course.json");
}

void LearnerController::attach(lq::Database *database)
{
    m_database = database;
    load();
}

lq::Date LearnerController::day() const
{
    return toLqDate(m_today);
}

void LearnerController::setError(const QString &error)
{
    if (m_error == error)
        return;
    m_error = error;
    emit errorChanged();
}

void LearnerController::load()
{
    m_state.reset();
    const auto &profile = m_profile->profile();
    if (!profile || !m_course) {
        emit changed();
        return;
    }
    lq::Plan plan;
    plan.days = profile->planDays;
    plan.start = toLqDate(profile->planStart);
    plan.studyDays = std::set<int>(profile->studyDays.begin(), profile->studyDays.end());
    plan.skipHolidays = profile->skipHolidays;
    for (const QDate &date : profile->daysOff)
        plan.daysOff.insert(toLqDate(date));

    if (m_demo) {
        m_state = lq::demoState(day(), m_course->unitWordIds());
    } else if (m_database) {
        QString error;
        m_state = lq::loadLearner(m_database->connection(), plan, &error);
        if (!m_state) {
            // Keep the file untouched: with no database attached, nothing below is saved.
            m_database = nullptr;
            setError(QStringLiteral("Your progress could not be read, so changes will not be saved. %1").arg(error));
            m_state = lq::defaultState(plan.start);
            m_state->plan = plan;
        }
    } else {
        m_state = lq::defaultState(plan.start);
        m_state->plan = plan;
    }

    // As when the web app opens: freezes cover missed study days, then the review task syncs. Achievements a
    // learner from before 0.4 already qualifies for are recorded without a toast each.
    lq::Events events = lq::applyFreezes(*m_state, day());
    const lq::ReviewSync sync = lq::syncReview(*m_state, day());
    events.insert(events.end(), sync.events.begin(), sync.events.end());
    const bool earned = !lq::checkAchievements(*m_state, day()).empty();
    if (!events.empty() || sync.changed || earned)
        save();
    announce(events);
    emit changed();
}

void LearnerController::save()
{
    if (m_demo || !m_database || !m_state)
        return;
    QString error;
    if (!lq::saveLearner(m_database->connection(), *m_state, &error))
        setError(QStringLiteral("Your progress could not be saved: %1").arg(error));
}

void LearnerController::apply(const std::function<lq::Events(lq::LearnerState &)> &change)
{
    if (!m_state)
        return;
    lq::Events events = change(*m_state);
    const lq::ReviewSync sync = lq::syncReview(*m_state, day());
    events.insert(events.end(), sync.events.begin(), sync.events.end());
    const std::vector<lq::Achievement> earned = lq::checkAchievements(*m_state, day());
    save();
    announce(events);
    for (const lq::Achievement &a : earned)
        emit celebrated(QStringLiteral("Achievement: ") + qs(a.title), qs(a.how), qs(a.icon));
    emit changed();
}

bool LearnerController::completeTask(lq::Task task)
{
    if (!m_state)
        return false;
    lq::Events events;
    apply([&](lq::LearnerState &state) {
        events = lq::markTask(state, task, day());
        return events;
    });
    const lq::PlanStatus status = lq::planStatus(*m_state);
    return !events.empty() || (!status.finished && lq::taskDone(*m_state, status.n, task));
}

QVariantMap LearnerController::tipMap(const lq::CoachTip &tip) const
{
    QString route;
    if (!tip.trainer.empty())
        route = QStringLiteral("practice/") + qs(tip.trainer);
    else if (tip.unit >= 0)
        route = QStringLiteral("lesson/%1/practice").arg(tip.unit);
    else if (tip.task)
        route = routeForTask(qs(lq::taskId(*tip.task)));
    return {
        {QStringLiteral("id"), qs(tip.id)},
        {QStringLiteral("title"), qs(tip.title)},
        {QStringLiteral("detail"), qs(tip.detail)},
        {QStringLiteral("route"), route},
    };
}

QVariantList LearnerController::coach(const QString &after) const
{
    QVariantList tips;
    if (!m_course || !m_state)
        return tips;
    for (const lq::CoachTip &tip : lq::coachTips(*m_state, *m_course, day(), after.toStdString()))
        tips.append(tipMap(tip));
    return tips;
}

QVariantMap LearnerController::nextStep(const QString &after) const
{
    const QVariantList tips = coach(after);
    for (const QVariant &tip : tips) {
        if (tip.toMap().value(QStringLiteral("id")).toString() == "plan"_L1)
            return tip.toMap();
    }
    return tips.isEmpty() ? QVariantMap() : tips.first().toMap();
}

bool LearnerController::reflectionDue() const
{
    return m_state && lq::reflectionDue(*m_state);
}

QString LearnerController::tasteText() const
{
    return m_state && !m_state->reflections.empty() ? qs(lq::tasteSummary(lq::taste(*m_state))) : QString();
}

QVariantList LearnerController::kindList(const QStringList &kinds)
{
    QVariantList out;
    for (const QString &kind : kinds) {
        QString label = qs(lq::kindLabel(kind.toStdString()));
        if (!label.isEmpty())
            label[0] = label[0].toUpper();
        out.append(QVariantMap{{QStringLiteral("id"), kind}, {QStringLiteral("label"), label}});
    }
    return out;
}

QVariantList LearnerController::exerciseKinds() const
{
    // What the learner meets most first.
    return kindList({QStringLiteral("listen"), QStringLiteral("order"), QStringLiteral("speak"), QStringLiteral("type"),
                     QStringLiteral("fill"), QStringLiteral("conj"), QStringLiteral("choice"), QStringLiteral("match"),
                     QStringLiteral("gender"), QStringLiteral("card"), QStringLiteral("describe")});
}

void LearnerController::countSession()
{
    apply([](lq::LearnerState &state) {
        lq::countSession(state);
        return lq::Events{};
    });
}

QString LearnerController::reflect(const QString &activity, const QStringList &offered, const QStringList &liked,
                                   const QString &feel, const QString &note)
{
    if (!m_state)
        return {};
    lq::Reflection reflection;
    reflection.date = day();
    reflection.activity = activity.toStdString();
    for (const QString &kind : offered)
        reflection.offered.push_back(kind.toStdString());
    for (const QString &kind : liked)
        reflection.liked.push_back(kind.toStdString());
    reflection.feel = lq::feelFromId(feel.toStdString());
    reflection.note = note.simplified().toStdString();
    apply([&reflection](lq::LearnerState &state) {
        lq::addReflection(state, std::move(reflection));
        return lq::Events{};
    });
    return qs(lq::tasteSummary(lq::taste(*m_state)));
}

void LearnerController::postponeReflection()
{
    apply([](lq::LearnerState &state) {
        lq::postponeReflection(state);
        return lq::Events{};
    });
}

void LearnerController::announce(const lq::Events &events)
{
    for (const lq::Event &e : events) {
        switch (e.type) {
        case lq::EventType::Streak:
            emit celebrated(QStringLiteral("%1-day streak!").arg(e.value), QStringLiteral("You earned your 10 XP for today."),
                            QStringLiteral("streak"));
            break;
        case lq::EventType::FreezeEarned:
            emit celebrated(QStringLiteral("Streak freeze earned"),
                            QStringLiteral("It covers a missed study day for you automatically."), {});
            break;
        case lq::EventType::FreezeUsed:
            emit celebrated(QStringLiteral("A streak freeze saved your streak"),
                            plural(e.value, QStringLiteral("It covered 1 missed study day."),
                                   QStringLiteral("It covered %1 missed study days.")),
                            {});
            break;
        case lq::EventType::DayComplete:
            emit celebrated(QStringLiteral("Day %1 complete!").arg(e.value), QStringLiteral("Every task of the day is done."), {});
            break;
        case lq::EventType::PhaseComplete:
            emit celebrated(QStringLiteral("Phase %1 complete!").arg(e.value), QStringLiteral("The next phase is open."), {});
            break;
        }
    }
}

int LearnerController::dueCount() const
{
    return m_state ? static_cast<int>(lq::reviewQueue(m_state->cards, day(), std::numeric_limits<int>::max()).size()) : 0;
}

int LearnerController::deckSize() const
{
    return m_state ? static_cast<int>(m_state->cards.size()) : 0;
}

QVariantList LearnerController::courseMap() const
{
    QVariantList phases;
    if (!m_course || !m_state)
        return phases;
    const lq::PlanStatus status = lq::planStatus(*m_state);
    const int todayUnit = !status.finished && status.entry.type == lq::DayType::Lesson ? status.entry.unit : -1;
    for (const lq::Phase &phase : m_course->phases) {
        QVariantList units;
        int done = 0;
        for (std::size_t i = 0; i < m_course->units.size(); ++i) {
            const lq::Unit &u = m_course->units[i];
            if (u.phase != phase.n)
                continue;
            const auto result = m_state->units.find(static_cast<int>(i));
            const bool finished = result != m_state->units.end();
            done += finished ? 1 : 0;
            units.append(QVariantMap{
                {QStringLiteral("index"), static_cast<int>(i)},
                {QStringLiteral("n"), u.n},
                {QStringLiteral("title"), qs(u.title)},
                {QStringLiteral("titleEs"), qs(u.titleEs)},
                {QStringLiteral("canDo"), qs(u.canDo)},
                {QStringLiteral("state"), finished ? QStringLiteral("done")
                                          : static_cast<int>(i) == todayUnit ? QStringLiteral("today") : QString()},
                {QStringLiteral("best"), finished ? percent(result->second.best) : -1},
            });
        }
        const auto checkpoint = m_state->checkpoints.find(phase.n);
        phases.append(QVariantMap{
            {QStringLiteral("n"), phase.n},
            {QStringLiteral("title"), qs(phase.title)},
            {QStringLiteral("titleEs"), qs(phase.titleEs)},
            {QStringLiteral("description"), qs(phase.description)},
            {QStringLiteral("done"), done},
            {QStringLiteral("units"), units},
            {QStringLiteral("checkpointBest"), checkpoint != m_state->checkpoints.end() ? percent(checkpoint->second.best) : -1},
        });
    }
    return phases;
}

QVariantMap LearnerController::unit(int index) const
{
    if (!m_course || index < 0 || index >= static_cast<int>(m_course->units.size()))
        return {};
    const lq::Unit &u = m_course->units[static_cast<std::size_t>(index)];
    QVariantList vocab, tables, examples, dialogue;
    for (const lq::Vocab &v : u.vocab)
        vocab.append(wordMap(v));
    for (const lq::GrammarTable &t : u.grammar.tables) {
        QVariantList rows;
        for (const auto &row : t.rows)
            rows.append(QVariant(qsList(row)));
        tables.append(QVariantMap{{QStringLiteral("head"), qsList(t.head)}, {QStringLiteral("rows"), rows}});
    }
    for (const auto &[es, en] : u.grammar.examples)
        examples.append(QVariantMap{{QStringLiteral("es"), qs(es)}, {QStringLiteral("en"), qs(en)}});
    for (const lq::DialogueLine &line : u.dialogue) {
        dialogue.append(QVariantMap{{QStringLiteral("speaker"), qs(line.speaker)},
                                    {QStringLiteral("es"), qs(line.es)},
                                    {QStringLiteral("en"), qs(line.en)},
                                    {QStringLiteral("me"), line.speaker == "Tú"},
                                    {QStringLiteral("voice"), qs(lq::voiceFor(line.speaker))}});
    }
    return {
        {QStringLiteral("index"), index},
        {QStringLiteral("id"), qs(u.id)},
        {QStringLiteral("n"), u.n},
        {QStringLiteral("phase"), u.phase},
        {QStringLiteral("phaseTitle"), qs(m_course->phases.at(static_cast<std::size_t>(u.phase - 1)).title)},
        {QStringLiteral("title"), qs(u.title)},
        {QStringLiteral("titleEs"), qs(u.titleEs)},
        {QStringLiteral("canDo"), qs(u.canDo)},
        {QStringLiteral("trick"), u.trick},
        {QStringLiteral("trickName"), trickName(u.trick)},
        {QStringLiteral("coach"), qs(u.coach)},
        {QStringLiteral("vocab"), vocab},
        {QStringLiteral("grammar"), QVariantMap{{QStringLiteral("title"), qs(u.grammar.title)},
                                                {QStringLiteral("hack"), qs(u.grammar.hack)},
                                                {QStringLiteral("tip"), qs(u.grammar.tip)},
                                                {QStringLiteral("tables"), tables},
                                                {QStringLiteral("examples"), examples}}},
        {QStringLiteral("dialogue"), dialogue},
        {QStringLiteral("speak"), qsList(u.speak)},
        {QStringLiteral("best"), bestPercent(m_state, &lq::LearnerState::units, index)},
        {QStringLiteral("next"), index + 1 < static_cast<int>(m_course->units.size()) ? index + 1 : -1},
    };
}

QVariantMap LearnerController::finishLesson(int unit, double score, int phrasesSaid)
{
    if (!m_course || !m_state || unit < 0 || unit >= static_cast<int>(m_course->units.size()))
        return {};
    const std::size_t before = m_state->cards.size();
    const bool firstTime = !m_state->units.contains(unit);
    const std::vector<std::string> words = m_course->unitWordIds().at(static_cast<std::size_t>(unit));
    apply([&](lq::LearnerState &state) {
        // Phrases said in the speaking step count for the day, as on the web; the lesson bonus is the reward.
        state.activity[day()].speak += phrasesSaid;
        return lq::learnUnit(state, unit, words, score, day());
    });
    return {
        {QStringLiteral("newCards"), static_cast<int>(m_state->cards.size() - before)},
        {QStringLiteral("firstTime"), firstTime},
    };
}

QVariantMap LearnerController::trainer(const QString &id) const
{
    QVariantMap t{{QStringLiteral("id"), id}};
    if (!m_course)
        return t;
    const lq::Course &c = *m_course;
    const auto set = [&t](const QString &title, const QString &description, int trick, bool automatic = false) {
        t[QStringLiteral("title")] = title;
        t[QStringLiteral("description")] = description;
        t[QStringLiteral("trick")] = trick;
        t[QStringLiteral("auto")] = automatic;
    };
    if (id == "verbs"_L1) {
        set(QStringLiteral("Verb gym · 80/20"),
            QStringLiteral("ser, estar, tener, hacer, poder, querer, ir, decir — in the tenses you have unlocked."), 1);
        QVariantList tenses;
        for (const lq::Tense tense : m_state ? lq::unlockedTenses(*m_state) : std::vector<lq::Tense>{lq::Tense::Present}) {
            const lq::TenseInfo &info = lq::tenseInfo(tense);
            tenses.append(QVariantMap{{QStringLiteral("en"), qs(info.en)}, {QStringLiteral("cue"), qs(info.cue)}});
        }
        t[QStringLiteral("tenses")] = tenses;
    } else if (id == "past"_L1) {
        set(QStringLiteral("Fui vs. Iba"), QStringLiteral("Pretérito or imperfecto? The skill that separates A2 from B1."), 5);
        QVariantList reasons;
        for (const auto &[key, reason] : c.pastReasons)
            reasons.append(qs(reason));
        t[QStringLiteral("reasons")] = reasons;
    } else if (id == "cognates"_L1) {
        set(QStringLiteral("COGNATES LAB"), QStringLiteral("Turn English words into Spanish with a handful of ending rules."), 1);
        QVariantList rules;
        for (const lq::CognateRule &r : c.cognateRules) {
            QStringList examples;
            for (const auto &[en, es] : r.examples)
                examples << qs(en) + QStringLiteral(" → ") + qs(es);
            rules.append(QVariantMap{{QStringLiteral("rule"), qs(r.en) + QStringLiteral(" → ") + qs(r.es)},
                                     {QStringLiteral("examples"), examples.join(QStringLiteral(", "))}});
        }
        t[QStringLiteral("rules")] = rules;
    } else if (id == "falsefriends"_L1) {
        set(QStringLiteral("False friends"), QStringLiteral("Words that look English but mean something else — embarazada!"), 1);
        QVariantList words;
        for (const lq::FalseFriend &f : c.falseFriends) {
            words.append(QVariantMap{{QStringLiteral("es"), qs(f.es)},
                                     {QStringLiteral("meaning"), qs(f.meaning)},
                                     {QStringLiteral("looksLike"), qs(f.looksLike)},
                                     {QStringLiteral("realWord"), qs(f.realWord)}});
        }
        t[QStringLiteral("falseFriends")] = words;
    } else if (id == "gender"_L1) {
        set(QStringLiteral("El / La trainer"),
            QStringLiteral("Never learn “mapa” — learn “el mapa”. Rules and famous exceptions."), 5);
        QVariantList rules;
        for (const lq::GenderRule &r : c.genderRules) {
            rules.append(QVariantMap{{QStringLiteral("rule"), qs(r.rule)},
                                     {QStringLiteral("examples"), qs(r.examples)},
                                     {QStringLiteral("note"), qs(r.note)}});
        }
        t[QStringLiteral("rules")] = rules;
    } else if (id == "speak"_L1) {
        set(QStringLiteral("Speaking: shadowing"),
            QStringLiteral("Listen, then repeat out loud 2–3 times. Train your mouth every day."), 4);
        const lq::PronunciationGroup &g = c.pronunciation.at(static_cast<std::size_t>(lq::pronunciationGroupFor(c, day())));
        t[QStringLiteral("group")] = QVariantMap{{QStringLiteral("title"), qs(g.title)},
                                                 {QStringLiteral("tip"), qs(g.tip)},
                                                 {QStringLiteral("items"), qsList(g.items)}};
    } else if (id == "describe"_L1) {
        set(QStringLiteral("Describe it in Spanish"),
            QStringLiteral("Don't know a word? Describe it. The core language-island skill."), 2);
        t[QStringLiteral("frames")] = qsList(c.describeFrames);
    } else if (id == "mixed"_L1) {
        set(QStringLiteral("Mixed practice"), QStringLiteral("Exercises from every unit you have completed, shuffled."), 3, true);
    } else if (id == "review"_L1) {
        set(QStringLiteral("Review"), QStringLiteral("Spaced repetition: words come back just before you forget them."), 1, true);
        std::optional<lq::Date> next;
        if (m_state) {
            for (const auto &[cardId, card] : m_state->cards) {
                if (!next || card.due < *next)
                    next = card.due;
            }
        }
        const QLocale en(QLocale::English, QLocale::UnitedStates);
        t[QStringLiteral("nextDue")] = next ? en.toString(toQDate(*next), QStringLiteral("MMM d")) : QString();
    }
    return t;
}

QVariantMap LearnerController::checkpoint(int phase) const
{
    if (!m_course || phase < 1 || phase > lq::kPhases)
        return {};
    return {
        {QStringLiteral("phase"), phase},
        {QStringLiteral("title"), QStringLiteral("Phase %1 checkpoint").arg(phase)},
        {QStringLiteral("phaseTitle"), qs(m_course->phases.at(static_cast<std::size_t>(phase - 1)).title)},
        {QStringLiteral("best"), bestPercent(m_state, &lq::LearnerState::checkpoints, phase)},
    };
}

QVariantMap LearnerController::mockExam() const
{
    if (!m_course)
        return {};
    const lq::MockExam &m = m_course->mock;
    QVariantList speaking;
    for (const lq::SpeakingTask &task : m.speaking) {
        speaking.append(QVariantMap{{QStringLiteral("title"), qs(task.title)},
                                    {QStringLiteral("prompt"), qs(task.prompt)},
                                    {QStringLiteral("image"), qs(task.image)}});
    }
    return {
        {QStringLiteral("readingText"), qs(m.reading.text)},
        {QStringLiteral("listeningText"), qs(m.listening.text)},
        {QStringLiteral("writingPrompt"), qs(m.writingPrompt)},
        {QStringLiteral("speaking"), speaking},
        {QStringLiteral("selfChecks"), QStringList{QStringLiteral("I greeted my friend and closed the email"),
                                                   QStringLiteral("I described a place or how I felt"),
                                                   QStringLiteral("I re-read it for el/la and -o/-a agreement")}},
        {QStringLiteral("rubric"), QStringList{QStringLiteral("Could not do it"), QStringLiteral("Long pauses, some English"),
                                               QStringLiteral("Got the message across with errors"),
                                               QStringLiteral("Fairly fluent, varied tenses")}},
        {QStringLiteral("best"), bestPercent(m_state, &lq::LearnerState::checkpoints, lq::kPhases)},
    };
}

QVariantList LearnerController::writingChecks(const QString &text) const
{
    QVariantList out;
    for (const lq::WritingCheck &check : lq::writingChecks(text.toStdString()))
        out.append(QVariantMap{{QStringLiteral("ok"), check.ok}, {QStringLiteral("label"), qs(check.label)}});
    return out;
}

QVariantMap LearnerController::finishMock(const QVariantMap &scores)
{
    const auto tally = [&scores](const char *key) {
        const QVariantList pair = scores.value(QString::fromLatin1(key)).toList();
        return pair.size() == 2 ? lq::Tally{pair[0].toInt(), pair[1].toInt()} : lq::Tally{};
    };
    lq::MockScores s;
    s.reading = tally("reading");
    s.listening = tally("listening");
    s.grammar = tally("grammar");
    s.writingPoints = scores.value(QStringLiteral("writing")).toInt();
    for (const QVariant &rating : scores.value(QStringLiteral("speaking")).toList())
        s.speaking.push_back(std::clamp(rating.toInt(), 0, lq::kSpeakingRubricMax));
    const lq::MockResult r = lq::mockResult(s);
    apply([&](lq::LearnerState &state) { return lq::saveCheckpoint(state, lq::kPhases, r.overall, day()); });
    return {
        {QStringLiteral("reading"), percent(r.reading)},
        {QStringLiteral("listening"), percent(r.listening)},
        {QStringLiteral("writing"), percent(r.writing)},
        {QStringLiteral("speaking"), percent(r.speaking)},
        {QStringLiteral("written"), percent(r.written)},
        {QStringLiteral("oral"), percent(r.oral)},
        {QStringLiteral("overall"), percent(r.overall)},
        {QStringLiteral("pass"), r.pass},
    };
}

QString LearnerController::routeForTask(const QString &taskId) const
{
    if (!m_state)
        return QStringLiteral("today");
    const lq::ScheduleEntry entry = lq::planStatus(*m_state).entry;
    if (taskId == "lesson"_L1)
        return QStringLiteral("lesson/%1").arg(entry.unit);
    if (taskId == "checkpoint"_L1)
        return QStringLiteral("checkpoint/%1").arg(entry.phase);
    if (taskId == "final"_L1)
        return QStringLiteral("mock");
    return QStringLiteral("practice/") + taskId;
}
