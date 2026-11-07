// The screens to browse (phase 4): progress, immersion and vocabulary. Every number comes from recorded
// activity through the engine; nothing here is a sample.
#include "learnercontroller.h"
#include "qtdate.h"

#include "lq/achievements.h"
#include "lq/calendar.h"
#include "lq/text.h"

#include <QLocale>

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>

namespace {

QString qs(std::string_view s)
{
    return QString::fromUtf8(s.data(), static_cast<qsizetype>(s.size()));
}

QString hours(double minutes)
{
    return QString::number(minutes / 60.0, 'f', 1);
}

QString shortDate(lq::Date date)
{
    return QLocale(QLocale::English, QLocale::UnitedStates).toString(toQDate(date), QStringLiteral("MMM d"));
}

QString studyDays(int n)
{
    return n == 1 ? QStringLiteral("1 study day") : QStringLiteral("%1 study days").arg(n);
}

// Pace needs a few study days behind it before it says anything.
constexpr int kPaceMinimumDays = 3;
// The weekly charts reach back at most this far.
constexpr int kChartWeeks = 12;

// The web app's skills, in its order.
constexpr std::pair<const char *, const char *> kSkills[]{
    {"verbs", "Verb forms"}, {"past", "Fui vs. iba"},   {"grammar", "Grammar"},   {"vocab", "Vocabulary"},
    {"listening", "Listening"}, {"gender", "El / la"}, {"cognates", "Cognates"}, {"falsefriends", "False friends"},
    {"review", "Flashcards"},
};

QString kindLabel(std::string_view kind)
{
    if (kind == "podcast")
        return QStringLiteral("Podcast");
    if (kind == "video")
        return QStringLiteral("Video");
    if (kind == "series")
        return QStringLiteral("Series or film");
    if (kind == "reading")
        return QStringLiteral("Reading");
    if (kind == "music")
        return QStringLiteral("Music");
    if (kind == "island")
        return QStringLiteral("Language island");
    return QStringLiteral("Other");
}

} // namespace

QVariantMap LearnerController::progressReport() const
{
    if (!m_state || !m_course)
        return {};
    const lq::LearnerState &state = *m_state;
    const lq::Date today = day();
    const lq::Totals t = lq::totals(state, today);
    const lq::StreakInfo streak = lq::streakInfo(state, today);
    const lq::PlanStatus status = lq::planStatus(state);

    QVariantList minutes;
    int most = 0;
    const QLocale en(QLocale::English, QLocale::UnitedStates);
    for (int i = -13; i <= 0; ++i) {
        const lq::Date date = lq::addDays(today, i);
        const int value = lq::spanishMinutes(state, date);
        most = std::max(most, value);
        minutes.append(QVariantMap{
            {QStringLiteral("day"), toQDate(date).day()},
            {QStringLiteral("label"), en.toString(toQDate(date), QStringLiteral("ddd, MMM d"))},
            {QStringLiteral("minutes"), value},
            {QStringLiteral("today"), i == 0},
        });
    }

    QVariantList skills;
    for (const auto &[id, label] : kSkills) {
        const auto accuracy = t.accuracy(id);
        skills.append(QVariantMap{
            {QStringLiteral("id"), QString::fromLatin1(id)},
            {QStringLiteral("label"), QString::fromUtf8(label)},
            {QStringLiteral("accuracy"), accuracy ? static_cast<int>(std::lround(*accuracy * 100)) : -1},
            {QStringLiteral("answered"), t.answered(id)},
        });
    }

    QVariantList readiness;
    int ready = 0;
    for (const lq::ReadinessSignal &signal : lq::readiness(state, t)) {
        ready += signal.met ? 1 : 0;
        readiness.append(QVariantMap{{QStringLiteral("met"), signal.met}, {QStringLiteral("text"), qs(signal.text)}});
    }

    QVariantList achievements;
    for (const lq::Achievement &a : lq::achievementList()) {
        const auto earned = state.achievements.find(a.id);
        achievements.append(QVariantMap{
            {QStringLiteral("id"), qs(a.id)},
            {QStringLiteral("icon"), qs(a.icon)},
            {QStringLiteral("title"), qs(a.title)},
            {QStringLiteral("how"), qs(a.how)},
            {QStringLiteral("earned"), earned != state.achievements.end()},
            {QStringLiteral("when"), earned != state.achievements.end() ? shortDate(earned->second) : QString()},
        });
    }

    // The goal, the date the plan ends at one plan day per study day, and the pace against the calendar.
    const lq::Plan &plan = state.plan;
    const lq::Pace pace = lq::pace(state, today);
    QVariantMap paceMap{
        {QStringLiteral("enough"), status.finished || pace.expected >= kPaceMinimumDays},
        {QStringLiteral("diff"), pace.diff},
    };
    if (status.finished) {
        paceMap[QStringLiteral("text")] = QStringLiteral("Plan complete");
        paceMap[QStringLiteral("tone")] = QStringLiteral("done");
        paceMap[QStringLiteral("detail")] = QStringLiteral("All %1 plan days done.").arg(plan.days);
    } else if (pace.expected < kPaceMinimumDays) {
        paceMap[QStringLiteral("text")] = QStringLiteral("Not enough data");
        paceMap[QStringLiteral("tone")] = QStringLiteral("none");
        paceMap[QStringLiteral("detail")] = QStringLiteral("Your pace shows after your first %1.").arg(studyDays(kPaceMinimumDays));
    } else {
        paceMap[QStringLiteral("text")] = pace.diff == 0 ? QStringLiteral("On track")
                                          : studyDays(std::abs(pace.diff)) + (pace.diff > 0 ? QStringLiteral(" ahead")
                                                                                            : QStringLiteral(" behind"));
        paceMap[QStringLiteral("tone")] = pace.diff >= 0 ? QStringLiteral("good") : QStringLiteral("behind");
        paceMap[QStringLiteral("detail")] = QStringLiteral("%1 of %2 plan days done; %3 expected by yesterday.")
                                                .arg(pace.doneCount).arg(plan.days).arg(pace.expected);
    }
    const QVariantMap goal{
        {QStringLiteral("days"), plan.days},
        {QStringLiteral("started"), QLocale(QLocale::English, QLocale::UnitedStates)
                                        .toString(toQDate(plan.start), QStringLiteral("MMM d, yyyy"))},
        {QStringLiteral("target"), pace.target ? en.toString(toQDate(*pace.target), QStringLiteral("MMM d, yyyy")) : QString()},
        {QStringLiteral("targetDetail"), status.finished ? QStringLiteral("The day you finished the plan")
                                         : QStringLiteral("%1 left, at one plan day per study day").arg(studyDays(pace.remaining))},
    };

    // The five phases with the plan days each one takes in this plan.
    const std::vector<lq::ScheduleEntry> &days = lq::schedule(plan.days);
    QVariantList roadmap;
    for (const lq::Phase &phase : m_course->phases) {
        int first = 0;
        int last = 0;
        for (const lq::ScheduleEntry &entry : days) {
            if (entry.phase != phase.n)
                continue;
            first = first == 0 ? entry.n : first;
            last = entry.n;
        }
        QString phaseState = QStringLiteral("upcoming");
        if (status.finished || state.checkpoints.contains(phase.n))
            phaseState = QStringLiteral("done");
        else if (phase.n == status.entry.phase)
            phaseState = QStringLiteral("current");
        roadmap.append(QVariantMap{
            {QStringLiteral("n"), phase.n},
            {QStringLiteral("title"), qs(phase.title)},
            {QStringLiteral("description"), qs(phase.description)},
            {QStringLiteral("state"), phaseState},
            {QStringLiteral("days"), first > 0 ? QStringLiteral("Day %1 – %2").arg(first).arg(last) : QString()},
        });
    }

    // Week by week, Monday to Sunday: Spanish minutes, and the words in the deck by the end of the week.
    // From the week the learner started (or the first recorded day), at most kChartWeeks back.
    lq::Date earliest = plan.start;
    if (!state.activity.empty())
        earliest = std::min(earliest, state.activity.begin()->first);
    for (const auto &[id, card] : state.cards)
        earliest = std::min(earliest, card.added);
    const lq::Date thisWeek = lq::weekStart(today);
    lq::Date firstWeek = std::max(lq::weekStart(std::min(earliest, today)), lq::addDays(thisWeek, -7 * (kChartWeeks - 1)));
    QVariantList weeks;
    int weekMinutes = 0;
    int mostWeek = 0;
    for (lq::Date start = firstWeek; start <= thisWeek; start = lq::addDays(start, 7)) {
        const lq::Date end = lq::addDays(start, 6);
        int total = 0;
        for (int i = 0; i < 7; ++i)
            total += lq::spanishMinutes(state, lq::addDays(start, i));
        const auto words = std::count_if(state.cards.begin(), state.cards.end(),
                                         [end](const auto &entry) { return entry.second.added <= end; });
        weekMinutes += total;
        mostWeek = std::max(mostWeek, total);
        weeks.append(QVariantMap{
            {QStringLiteral("label"), shortDate(start)},
            {QStringLiteral("range"), shortDate(start) + QStringLiteral(" – ") + shortDate(end)},
            {QStringLiteral("minutes"), total},
            {QStringLiteral("words"), static_cast<int>(words)},
            {QStringLiteral("current"), start == thisWeek},
        });
    }
    int spoken = 0;
    for (const auto &[date, activity] : state.activity)
        spoken += activity.speak;

    return {
        {QStringLiteral("goal"), goal},
        {QStringLiteral("pace"), paceMap},
        {QStringLiteral("roadmap"), roadmap},
        {QStringLiteral("weeks"), weeks},
        {QStringLiteral("mostWeekMinutes"), mostWeek},
        {QStringLiteral("weekAverage"), weeks.isEmpty() ? QString() : hours(static_cast<double>(weekMinutes) / weeks.size())},
        {QStringLiteral("deckWords"), static_cast<int>(state.cards.size())},
        {QStringLiteral("spoken"), spoken},
        {QStringLiteral("spanishHours"), hours(t.spanishMinutes())},
        {QStringLiteral("spanishDetail"), QStringLiteral("app %1 · listening %2 · island %3 · talk %4")
                                              .arg(hours(t.appMinutes), hours(t.inputMinutes), hours(t.islandMinutes),
                                                   hours(t.talkMinutes))},
        {QStringLiteral("lessons"), static_cast<int>(state.units.size())},
        {QStringLiteral("lessonCount"), static_cast<int>(m_course->units.size())},
        {QStringLiteral("planDone"), status.doneCount},
        {QStringLiteral("planTotal"), status.total},
        {QStringLiteral("wordsRemembered"), t.wordsRemembered},
        {QStringLiteral("wordsSeen"), t.wordsSeen},
        {QStringLiteral("streak"), streak.count},
        {QStringLiteral("longest"), streak.longest},
        {QStringLiteral("freezes"), streak.freezes},
        {QStringLiteral("minutes"), minutes},
        {QStringLiteral("mostMinutes"), most},
        {QStringLiteral("skills"), skills},
        {QStringLiteral("readiness"), readiness},
        {QStringLiteral("readyCount"), ready},
        {QStringLiteral("achievements"), achievements},
        {QStringLiteral("earnedCount"), static_cast<int>(state.achievements.size())},
    };
}

QVariantMap LearnerController::coachReport() const
{
    if (!m_state)
        return {};
    const lq::LearnerState &state = *m_state;
    const lq::Date today = day();

    // The same answers the coach weighs: those of the last kSkillWindow days.
    std::map<std::string, lq::Tally, std::less<>> recent;
    for (const auto &[date, activity] : state.activity) {
        if (date > today || lq::diffDays(date, today) >= lq::kSkillWindow)
            continue;
        for (const auto &[skill, tally] : activity.answers) {
            recent[skill].correct += tally.correct;
            recent[skill].total += tally.total;
        }
    }
    QVariantList skills;
    int answers = 0;
    for (const auto &[id, label] : kSkills) {
        const auto it = recent.find(id);
        const int correct = it == recent.end() ? 0 : it->second.correct;
        const int total = it == recent.end() ? 0 : it->second.total;
        answers += total;
        skills.append(QVariantMap{
            {QStringLiteral("id"), QString::fromLatin1(id)},
            {QStringLiteral("label"), QString::fromUtf8(label)},
            {QStringLiteral("correct"), correct},
            {QStringLiteral("total"), total},
            {QStringLiteral("accuracy"), total > 0 ? static_cast<int>(std::lround(100.0 * correct / total)) : -1},
            {QStringLiteral("enough"), total >= lq::kSkillMinAnswers},
        });
    }

    const lq::PlanStatus status = lq::planStatus(state);
    const lq::Pace pace = lq::pace(state, today);
    QString paceText;
    QString paceTone;
    if (status.finished) {
        paceText = QStringLiteral("Plan complete");
        paceTone = QStringLiteral("good");
    } else if (pace.expected < kPaceMinimumDays) {
        paceText = QStringLiteral("Not enough data yet");
        paceTone = QStringLiteral("none");
    } else {
        paceText = pace.diff == 0 ? QStringLiteral("On track")
                                  : studyDays(std::abs(pace.diff)) + (pace.diff > 0 ? QStringLiteral(" ahead")
                                                                                    : QStringLiteral(" behind"));
        paceTone = pace.diff >= 0 ? QStringLiteral("good") : QStringLiteral("behind");
    }

    QVariantList favourites;
    const lq::Taste learned = lq::taste(state);
    for (const std::string &kind : learned.favourites()) {
        QString label = qs(lq::kindLabel(kind));
        if (!label.isEmpty())
            label[0] = label[0].toUpper();
        favourites.append(label);
    }

    return {
        {QStringLiteral("skills"), skills},
        {QStringLiteral("answers"), answers},
        {QStringLiteral("windowDays"), lq::kSkillWindow},
        {QStringLiteral("minAnswers"), lq::kSkillMinAnswers},
        {QStringLiteral("weakBelow"), static_cast<int>(std::lround(lq::kWeakSkill * 100))},
        {QStringLiteral("due"), static_cast<int>(lq::reviewQueue(state.cards, today, std::numeric_limits<int>::max()).size())},
        {QStringLiteral("pace"), paceText},
        {QStringLiteral("paceTone"), paceTone},
        {QStringLiteral("favourites"), favourites},
        {QStringLiteral("reflections"), learned.reflections},
    };
}

QVariantMap LearnerController::immersion() const
{
    if (!m_state || !m_course)
        return {};
    const lq::LearnerState &state = *m_state;
    const lq::Date today = day();
    const lq::Totals t = lq::totals(state, today);
    const auto found = state.activity.find(today);
    const int todayMinutes = found == state.activity.end()
                                 ? 0
                                 : found->second.inputMinutes + found->second.islandMinutes + found->second.talkMinutes;
    const lq::ListeningLevel level = lq::listeningLevel(t.inputMinutes);

    QVariantList log;
    for (std::size_t i = state.input.size(); i-- > 0 && log.size() < 8;) {
        const lq::InputEntry &entry = state.input[i];
        log.append(QVariantMap{
            {QStringLiteral("index"), static_cast<int>(i)},
            {QStringLiteral("date"), shortDate(entry.date)},
            {QStringLiteral("minutes"), entry.minutes},
            {QStringLiteral("kind"), kindLabel(entry.kind)},
            {QStringLiteral("source"), qs(entry.source)},
            {QStringLiteral("imported"), entry.imported},
        });
    }
    QVariantList exchanges;
    for (std::size_t i = state.exchanges.size(); i-- > 0 && exchanges.size() < 4;) {
        const lq::Exchange &e = state.exchanges[i];
        exchanges.append(QVariantMap{
            {QStringLiteral("date"), shortDate(e.date)},
            {QStringLiteral("minutes"), e.minutes},
            {QStringLiteral("partner"), qs(e.partner)},
            {QStringLiteral("note"), qs(e.note)},
        });
    }
    QVariantList missions;
    int missionsDone = 0;
    for (const lq::Mission &m : m_course->immersion.missions) {
        const bool done = state.missions.contains(m.id);
        missionsDone += done ? 1 : 0;
        missions.append(QVariantMap{
            {QStringLiteral("id"), qs(m.id)},
            {QStringLiteral("title"), qs(m.title)},
            {QStringLiteral("how"), qs(m.how)},
            {QStringLiteral("done"), done},
        });
    }
    QVariantList resources;
    for (const lq::Resource &r : m_course->immersion.resources) {
        resources.append(QVariantMap{
            {QStringLiteral("name"), qs(r.name)},
            {QStringLiteral("level"), qs(r.level)},
            {QStringLiteral("accent"), qs(r.accent)},
            {QStringLiteral("kind"), qs(r.kind)},
            {QStringLiteral("url"), qs(r.url)},
            {QStringLiteral("note"), qs(r.note)},
        });
    }
    QVariantList tips;
    for (const std::string &tip : m_course->immersion.seriesTips)
        tips.append(qs(tip));

    // Series by level: the learner's own first; the next level opens with its checkpoint.
    const auto &profile = m_profile->profile();
    const std::string start = profile ? profile->level.toStdString() : std::string();
    const std::string seriesLevel = lq::seriesLevel(start, state);
    QVariantList series;
    for (const lq::Series &s : m_course->immersion.series) {
        QVariantList where;
        for (const lq::Series::Where &w : s.where)
            where.append(QVariantMap{{QStringLiteral("service"), qs(w.service)}, {QStringLiteral("detail"), qs(w.detail)}});
        series.append(QVariantMap{
            {QStringLiteral("name"), qs(s.name)},
            {QStringLiteral("level"), qs(s.level)},
            {QStringLiteral("years"), qs(s.years)},
            {QStringLiteral("kind"), qs(s.kind)},
            {QStringLiteral("note"), qs(s.note)},
            {QStringLiteral("where"), where},
        });
    }
    // Why this level, and what opens the next one.
    const QString seriesWhy = seriesLevel == "B1"
        ? QStringLiteral("You passed the phase %1 checkpoint, so B1 series are yours.").arg(lq::kSeriesB1Phase)
        : seriesLevel == "A2"
        ? QStringLiteral("Pass the phase %1 checkpoint with %2% and B1 series open up.")
              .arg(lq::kSeriesB1Phase).arg(qRound(lq::kCheckpointPass * 100))
        : QStringLiteral("Pass the phase %1 checkpoint with %2% and A2 series open up.")
              .arg(lq::kSeriesA2Phase).arg(qRound(lq::kCheckpointPass * 100));
    QVariantList phrases;
    for (const auto &[es, en] : m_course->immersion.exchangePhrases)
        phrases.append(QVariantMap{{QStringLiteral("es"), qs(es)}, {QStringLiteral("en"), qs(en)}});
    QVariantList kinds;
    for (const std::string_view kind : lq::inputKinds())
        kinds.append(QVariantMap{{QStringLiteral("id"), qs(kind)}, {QStringLiteral("label"), kindLabel(kind)}});

    return {
        {QStringLiteral("todayMinutes"), todayMinutes},
        {QStringLiteral("target"), lq::inputTargetMinutes(state.plan.days)},
        {QStringLiteral("listeningHours"), hours(t.inputMinutes)},
        {QStringLiteral("islandHours"), hours(t.islandMinutes)},
        {QStringLiteral("level"), level.level},
        {QStringLiteral("nextHours"), level.nextHours},
        {QStringLiteral("levelProgress"), level.progress},
        {QStringLiteral("log"), log},
        {QStringLiteral("exchanges"), exchanges},
        {QStringLiteral("exchangeCount"), static_cast<int>(state.exchanges.size())},
        {QStringLiteral("missions"), missions},
        {QStringLiteral("missionsDone"), missionsDone},
        {QStringLiteral("resources"), resources},
        {QStringLiteral("seriesTips"), tips},
        {QStringLiteral("series"), series},
        {QStringLiteral("seriesLevel"), qs(seriesLevel)},
        {QStringLiteral("seriesWhy"), seriesWhy},
        {QStringLiteral("seriesWhere"), qs(m_course->immersion.seriesWhere)},
        {QStringLiteral("phrases"), phrases},
        {QStringLiteral("kinds"), kinds},
        {QStringLiteral("maxMinutes"), lq::kMaxLoggedMinutes},
        {QStringLiteral("maxExchangeMinutes"), lq::kMaxExchangeMinutes},
        {QStringLiteral("maxText"), static_cast<int>(lq::kMaxLogText)},
    };
}

QVariantList LearnerController::vocabulary() const
{
    QVariantList words;
    if (!m_state || !m_course)
        return words;
    const lq::Date today = day();
    for (std::size_t u = 0; u < m_course->units.size(); ++u) {
        const lq::Unit &unit = m_course->units[u];
        for (const lq::Vocab &v : unit.vocab) {
            QString state = QStringLiteral("new");
            if (const auto card = m_state->cards.find(v.id); card != m_state->cards.end()) {
                if (lq::isDue(card->second, today))
                    state = QStringLiteral("due");
                else
                    state = lq::remembered(card->second) ? QStringLiteral("remembered") : QStringLiteral("learning");
            }
            QVariantMap word = wordMap(v);
            word.insert(QStringLiteral("unit"), static_cast<int>(u));
            word.insert(QStringLiteral("lesson"), unit.n);
            word.insert(QStringLiteral("lessonTitle"), qs(unit.title));
            word.insert(QStringLiteral("phase"), unit.phase);
            word.insert(QStringLiteral("state"), state);
            word.insert(QStringLiteral("key"), qs(lq::stripAccents(lq::normalize(v.es + " " + v.en))));
            words.append(word);
        }
    }
    return words;
}

QString LearnerController::searchKey(const QString &text) const
{
    return qs(lq::stripAccents(lq::normalize(text.toStdString())));
}

void LearnerController::logInput(int minutes, const QString &kind, const QString &source)
{
    apply([&](lq::LearnerState &state) {
        return lq::logInput(state, minutes, kind.toStdString(), source.toStdString(), day());
    });
}

void LearnerController::removeInput(int index)
{
    if (index < 0)
        return;
    apply([&](lq::LearnerState &state) {
        lq::removeInput(state, static_cast<std::size_t>(index));
        return lq::Events{};
    });
}

void LearnerController::logExchange(int minutes, const QString &partner, const QString &note)
{
    apply([&](lq::LearnerState &state) {
        return lq::logExchange(state, minutes, partner.toStdString(), note.toStdString(), day());
    });
}

void LearnerController::setMission(const QString &id, bool done)
{
    apply([&](lq::LearnerState &state) {
        lq::setMission(state, id.toStdString(), done, day());
        return lq::Events{};
    });
}
