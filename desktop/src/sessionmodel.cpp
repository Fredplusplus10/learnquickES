#include "sessionmodel.h"

#include <QRandomGenerator>

#include <algorithm>
#include <cmath>
#include <map>

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

QString typeName(lq::ItemType type)
{
    return qs(lq::itemKind(type));
}

// The instruction above each exercise, as in the web app.
QString instruction(lq::ItemType type)
{
    switch (type) {
    case lq::ItemType::Choice: return u"Choose the right answer"_s;
    case lq::ItemType::Fill: return u"Fill in the blank"_s;
    case lq::ItemType::Type: return u"Type the answer"_s;
    case lq::ItemType::Conjugate: return u"Conjugate the verb"_s;
    case lq::ItemType::Order: return u"Build the sentence in Spanish"_s;
    case lq::ItemType::Listen: return u"Listen and build what you hear"_s;
    case lq::ItemType::Match: return u"Match the pairs"_s;
    case lq::ItemType::Gender: return u"El or la? Learn the article with the noun"_s;
    case lq::ItemType::Card: return u"Review"_s;
    case lq::ItemType::Speak: return u"Say it out loud"_s;
    case lq::ItemType::Describe: return u"Describe it in Spanish — no English!"_s;
    }
    return {};
}

bool checkable(lq::ItemType type)
{
    switch (type) {
    case lq::ItemType::Choice:
    case lq::ItemType::Fill:
    case lq::ItemType::Type:
    case lq::ItemType::Conjugate:
    case lq::ItemType::Order:
    case lq::ItemType::Listen:
    case lq::ItemType::Gender:
        return true;
    default:
        return false;
    }
}

QVariantList indices(const std::vector<int> &list)
{
    QVariantList out;
    for (const int i : list)
        out.append(i);
    return out;
}

constexpr double kCheckpointPass = 0.7;

} // namespace

SessionModel::SessionModel(LearnerController *learner, quint32 seed, QObject *parent)
    : QObject(parent)
    , m_learner(learner)
    , m_rnd(seed ? seed : QRandomGenerator::global()->generate())
{
}

void SessionModel::setAudioAvailable(bool available)
{
    m_audio = available;
}

bool SessionModel::canCheck() const
{
    return onItem() && !m_checked && lq::canCheck(m_session.current(), m_response);
}

QVariantList SessionModel::picked() const
{
    return indices(m_response.picked);
}

QVariantList SessionModel::matched() const
{
    return indices(m_matched);
}

// ---- Starting ------------------------------------------------------------------------------------------

void SessionModel::startLesson(int unit)
{
    const lq::Course *course = m_learner->course();
    if (!course || !m_learner->ready() || unit < 0 || unit >= static_cast<int>(course->units.size()))
        return;
    begin(u"lesson"_s, QString::number(unit), lq::lessonItems(*course, unit, m_rnd), true);
}

void SessionModel::startTrainer(const QString &id)
{
    const lq::Course *course = m_learner->course();
    if (!course || !m_learner->ready())
        return;
    const lq::LearnerState &state = m_learner->state();
    const std::vector<int> done = lq::completedUnits(state);
    std::vector<lq::Item> items;
    if (id == "review"_L1)
        items = lq::cardItems(*course, lq::reviewQueue(state.cards, m_learner->day()));
    else if (id == "verbs"_L1)
        items = lq::conjugationItems(lq::coreVerbs(), lq::unlockedTenses(state), 10, false, m_rnd);
    else if (id == "mixed"_L1) {
        // Once the learner has said what they enjoy, ten of thirty candidates, drawn with their taste.
        const lq::Taste taste = lq::taste(state);
        items = taste.reflections == 0 ? lq::mixedItems(*course, done, 10, m_rnd)
                                       : lq::preferredItems(lq::mixedItems(*course, done, 30, m_rnd), 10, taste, m_rnd);
    } else if (id == "speak"_L1)
        items = lq::speakingItems(*course, done, m_learner->day(), m_rnd);
    else if (id == "cognates"_L1)
        items = lq::cognateItems(*course, 10, m_rnd);
    else if (id == "falsefriends"_L1)
        items = lq::falseFriendItems(*course, 8, m_rnd);
    else if (id == "gender"_L1)
        items = lq::genderItems(*course, 12, m_rnd);
    else if (id == "past"_L1)
        items = lq::pastItems(*course, 10, m_rnd);
    else if (id == "describe"_L1)
        items = lq::describeItems(*course, 5, m_rnd);
    else
        return;
    begin(u"trainer"_s, id, std::move(items), true);
}

void SessionModel::startCheckpoint(int phase)
{
    const lq::Course *course = m_learner->course();
    if (!course || !m_learner->ready() || phase < 1 || phase >= lq::kPhases)
        return;
    // No second tries: a checkpoint shows what is solid and what needs review.
    begin(u"checkpoint"_s, QString::number(phase), lq::checkpointItems(*course, phase, m_rnd), false);
}

void SessionModel::startMockPart(const QString &part)
{
    const lq::Course *course = m_learner->course();
    if (!course || !m_learner->ready())
        return;
    std::vector<lq::Item> items;
    if (part == "reading"_L1)
        items = lq::mockQuestions(course->mock.reading, "reading", m_rnd);
    else if (part == "listening"_L1)
        items = lq::mockQuestions(course->mock.listening, "listening", m_rnd);
    else if (part == "grammar"_L1)
        items = lq::mockGrammarItems(*course, m_rnd);
    else
        return;
    begin(u"mock"_s, part, std::move(items), false);
}

QVariantMap SessionModel::startAssessment(const QString &level, const QString &seed)
{
    const lq::Assessments *assessments = m_learner->assessments();
    const lq::LevelAssessment *assessment = assessments ? assessments->level(level.toStdString()) : nullptr;
    if (!assessment)
        return {};
    stop();
    const QByteArray seedText = seed.toUtf8();
    lq::SeededRandom drawRandom(std::string_view(seedText.constData(), static_cast<std::size_t>(seedText.size())));
    m_draw = lq::drawAssessment(*assessment, drawRandom);
    const lq::AssessmentPassage &reading = assessment->reading.at(static_cast<std::size_t>(m_draw.reading));
    const lq::AssessmentPassage &listening = assessment->listening.at(static_cast<std::size_t>(m_draw.listening));
    return {
        {u"level"_s, level},
        {u"readingText"_s, qs(reading.text)},
        {u"listeningText"_s, qs(listening.text)},
        {u"listeningIntro"_s, qs(listening.intro)},
        {u"reading"_s, static_cast<int>(reading.questions.size())},
        {u"listening"_s, static_cast<int>(listening.questions.size())},
        {u"language"_s, static_cast<int>(m_draw.language.size())},
    };
}

void SessionModel::startAssessmentPart(const QString &part)
{
    const lq::Assessments *assessments = m_learner->assessments();
    const lq::LevelAssessment *assessment = assessments ? assessments->level(m_draw.level) : nullptr;
    if (!assessment)
        return;
    std::vector<lq::Item> items = lq::assessmentItems(*assessment, m_draw, part.toStdString(), m_rnd);
    if (items.empty())
        return;
    begin(u"assessment"_s, part, std::move(items), false);
}

void SessionModel::stop()
{
    m_active = false;
    m_intermission = false;
    m_item.clear();
    emit changed();
}

void SessionModel::begin(const QString &kind, const QString &trainer, std::vector<lq::Item> items, bool requeue)
{
    // Without a Spanish voice, "listen and build" becomes "build the sentence" with the English shown.
    if (!m_audio) {
        for (lq::Item &item : items) {
            if (item.type == lq::ItemType::Listen)
                item.type = lq::ItemType::Order;
        }
    }
    // The kinds of exercise in the session, which the reflection asks about; a lesson also has its speaking step.
    m_kinds.clear();
    for (const lq::Item &item : items) {
        const QString name = typeName(item.type);
        if (!m_kinds.contains(name))
            m_kinds.append(name);
    }
    const bool speaks = kind == "lesson"_L1 && !m_learner->course()->units.at(trainer.toUInt()).speak.empty();
    if (speaks && !m_kinds.contains(u"speak"_s))
        m_kinds.append(u"speak"_s);
    m_session = lq::Session(std::move(items), requeue);
    m_active = true;
    m_intermission = false;
    m_kind = kind;
    m_trainer = trainer;
    m_result.clear();
    const auto &activity = m_learner->state().activity;
    const auto today = activity.find(m_learner->day());
    m_xpAtStart = today == activity.end() ? 0 : today->second.xp;
    if (m_session.finished())
        finish();
    else
        startItem();
}

void SessionModel::startItem()
{
    m_response = {};
    m_article.clear();
    m_checked = false;
    m_revealed = false;
    m_matchSide.clear();
    m_matchIndex = -1;
    m_matchMiss.clear();
    m_matched.clear();
    m_matchMistakes = 0;
    m_feedback.clear();
    m_item = itemMap(m_session.current());
    if (m_kind == "assessment"_L1)
        m_item.remove(u"answer"_s);  // a test's right option stays in C++
    if (m_session.current().type == lq::ItemType::Match) {
        // Each column in its own random order, so pairs never line up.
        std::vector<int> order(m_session.current().pairs.size());
        for (std::size_t i = 0; i < order.size(); ++i)
            order[i] = static_cast<int>(i);
        m_item[u"left"_s] = indices(lq::shuffled(order, m_rnd));
        m_item[u"right"_s] = indices(lq::shuffled(order, m_rnd));
    }
    emit changed();
}

QVariantMap SessionModel::itemMap(const lq::Item &item) const
{
    QVariantMap map{
        {u"key"_s, static_cast<int>(m_session.position())},
        {u"type"_s, typeName(item.type)},
        {u"instruction"_s, instruction(item.type)},
        {u"skill"_s, qs(item.skill)},
        {u"question"_s, qs(item.question)},
        {u"prompt"_s, qs(item.prompt)},
        {u"options"_s, qsList(item.options)},
        {u"answer"_s, item.answer},
        {u"hint"_s, qs(item.hint)},
        {u"audio"_s, qs(item.audio)},
        {u"es"_s, qs(item.es)},
        {u"en"_s, qs(item.en)},
        {u"tiles"_s, qsList(item.tiles)},
        {u"word"_s, qs(item.word)},
        {u"articles"_s, qsList(item.articles)},
        {u"example"_s, qs(item.example)},
        {u"gender"_s, item.gender ? QString(QChar::fromLatin1(item.gender)) : QString()},
        {u"model"_s, qs(item.model)},
        {u"pronoun"_s, qs(item.pronoun)},
        {u"retry"_s, item.retry},
    };
    if (item.type == lq::ItemType::Fill) {
        const qsizetype blank = map[u"question"_s].toString().indexOf(u"___"_s);
        const QString q = map[u"question"_s].toString();
        map[u"before"_s] = blank < 0 ? q : q.left(blank);
        map[u"after"_s] = blank < 0 ? QString() : q.mid(blank + 3);
    }
    if (item.type == lq::ItemType::Gender) {
        // Split "el mapa" into article and word for display.
        const QString full = qs(item.es);
        const qsizetype space = full.indexOf(u' ');
        map[u"article"_s] = space > 0 ? full.left(space) : QString();
    }
    if (item.type == lq::ItemType::Conjugate) {
        const lq::TenseInfo &tense = lq::tenseInfo(item.tense);
        const lq::Verb *verb = lq::findVerb(item.verb);
        map[u"verb"_s] = qs(item.verb);
        map[u"verbEn"_s] = verb ? qs(verb->en) : QString();
        map[u"tense"_s] = qs(tense.en);
        map[u"cue"_s] = qs(tense.cue);
        std::size_t longest = 5;
        for (const std::string &a : item.answers)
            longest = std::max(longest, lq::codePointCount(a));
        map[u"width"_s] = static_cast<int>(longest) + 2;
    }
    if (item.type == lq::ItemType::Fill) {
        std::size_t longest = 5;
        for (const std::string &a : item.answers)
            longest = std::max(longest, lq::codePointCount(a));
        map[u"width"_s] = static_cast<int>(longest) + 2;
    }
    if (item.type == lq::ItemType::Match) {
        QVariantList pairs;
        std::vector<int> order(item.pairs.size());
        for (std::size_t i = 0; i < item.pairs.size(); ++i) {
            pairs.append(QVariantMap{{u"es"_s, qs(item.pairs[i].first)}, {u"en"_s, qs(item.pairs[i].second)}});
            order[i] = static_cast<int>(i);
        }
        map[u"pairs"_s] = pairs;
        map[u"left"_s] = indices(order);  // startItem shuffles both columns
        map[u"right"_s] = indices(order);
    }
    if (item.type == lq::ItemType::Card) {
        const lq::Deck &deck = m_learner->state().cards;
        const auto stored = deck.find(item.cardId);
        const lq::Card card = stored != deck.end() ? stored->second : lq::newCard(m_learner->day());
        // Stronger cards are asked the other way round: English → Spanish.
        map[u"reverse"_s] = stored != deck.end() && card.box >= 3;
        const auto days = [&](lq::Grade g) {
            return lq::diffDays(m_learner->day(), lq::gradeCard(card, g, m_learner->day()).due);
        };
        map[u"goodDays"_s] = days(lq::Grade::Good);
        map[u"easyDays"_s] = days(lq::Grade::Easy);
        const QString full = qs(item.es);
        const qsizetype space = item.gender ? full.indexOf(u' ') : -1;
        map[u"article"_s] = space > 0 ? full.left(space) : QString();
        map[u"word"_s] = space > 0 ? full.mid(space + 1) : full;
    }
    if (item.type == lq::ItemType::Describe) {
        if (const lq::Course *course = m_learner->course())
            map[u"frames"_s] = qsList(course->describeFrames);
    }
    return map;
}

// ---- Interaction ---------------------------------------------------------------------------------------

void SessionModel::select(int option)
{
    if (!onItem() || m_checked || m_session.current().type != lq::ItemType::Choice
        || option < 0 || option >= static_cast<int>(m_session.current().options.size()))
        return;
    m_response.choice = option;
    emit changed();
}

void SessionModel::pick(int tile)
{
    if (!onItem() || m_checked || tile < 0
        || tile >= static_cast<int>(m_session.current().tiles.size())
        || std::find(m_response.picked.begin(), m_response.picked.end(), tile) != m_response.picked.end())
        return;
    m_response.picked.push_back(tile);
    emit changed();
}

void SessionModel::unpick(int tile)
{
    if (!onItem() || m_checked)
        return;
    std::erase(m_response.picked, tile);
    emit changed();
}

void SessionModel::setInput(const QString &text)
{
    if (!onItem() || m_checked)
        return;
    // Keyboards can produce decomposed accents; compare the composed form, as typed text normally is.
    m_response.text = text.normalized(QString::NormalizationForm_C).toStdString();
    emit changed();
}

void SessionModel::check()
{
    if (!canCheck())
        return;
    answer(lq::checkItem(m_session.current(), m_response));
}

void SessionModel::skip()
{
    if (!onItem() || m_checked || !checkable(m_session.current().type))
        return;
    const lq::Item &item = m_session.current();
    lq::AnswerCheck miss;
    miss.expected = lq::correctAnswer(item);
    if (item.type == lq::ItemType::Gender)
        m_article.clear();
    answer(miss);
}

void SessionModel::chooseArticle(const QString &article)
{
    if (!onItem() || m_checked || m_session.current().type != lq::ItemType::Gender)
        return;
    m_article = article;
    lq::AnswerCheck result;
    result.expected = m_session.current().article;
    result.ok = article.toStdString() == result.expected;
    answer(result);
}

void SessionModel::tapMatch(const QString &side, int index)
{
    if (!onItem() || m_checked || m_session.current().type != lq::ItemType::Match)
        return;
    if (index < 0 || index >= static_cast<int>(m_session.current().pairs.size())
        || std::find(m_matched.begin(), m_matched.end(), index) != m_matched.end())
        return;
    if (m_matchSide.isEmpty() || m_matchSide == side) {
        m_matchSide = side;
        m_matchIndex = index;
        m_matchMiss.clear();
        emit changed();
        return;
    }
    const lq::Item &item = m_session.current();
    if (m_matchIndex == index) {
        m_matched.push_back(index);
        emit sayRequested(qs(item.pairs[static_cast<std::size_t>(index)].first));
    } else {
        ++m_matchMistakes;
        m_matchMiss = {{m_matchSide, m_matchIndex}, {side, index}};
    }
    m_matchSide.clear();
    m_matchIndex = -1;
    if (m_matched.size() == item.pairs.size()) {
        lq::AnswerCheck result;
        result.ok = m_matchMistakes <= 1;
        answer(result);
    } else {
        emit changed();
    }
}

void SessionModel::answer(const lq::AnswerCheck &check)
{
    const lq::Item item = m_session.current();
    m_checked = true;
    const bool requeued = m_session.answer(check.ok, m_rnd);
    if (m_kind == "assessment"_L1) {
        // A test: the answer counts only towards the mark, and nothing tells whether it was right.
        advance();
        return;
    }
    m_learner->apply([&](lq::LearnerState &state) {
        return lq::recordAnswer(state, item.skill, check.ok, check.almost, item.retry, m_learner->day());
    });
    static const std::vector<std::string> praise{"¡Muy bien!", "¡Perfecto!", "¡Excelente!", "¡Eso es!", "¡Correcto!"};
    m_feedback = {
        {u"ok"_s, check.ok},
        {u"almost"_s, check.almost},
        {u"praise"_s, qs(lq::pickOne(praise, m_rnd))},
        {u"expected"_s, check.ok ? qs(check.expected) : qs(lq::correctAnswer(item))},
        {u"why"_s, qs(item.why)},
        {u"note"_s, qs(item.note)},
        {u"sentence"_s, qs(lq::sentenceOf(item))},
        {u"requeued"_s, requeued},
    };
    emit changed();
}

void SessionModel::reveal()
{
    if (!onItem() || m_revealed)
        return;
    const lq::ItemType type = m_session.current().type;
    if (type != lq::ItemType::Card && type != lq::ItemType::Describe)
        return;
    m_revealed = true;
    emit changed();
}

void SessionModel::selfRate(bool managed)
{
    if (!onItem())
        return;
    const lq::Item &item = m_session.current();
    if (item.type == lq::ItemType::Speak) {
        m_learner->apply([&](lq::LearnerState &state) { return lq::recordPhrase(state, m_learner->day()); });
    } else if (item.type == lq::ItemType::Describe && m_revealed) {
        m_learner->apply([&](lq::LearnerState &state) {
            return lq::recordAnswer(state, "describe", managed, false, false, m_learner->day());
        });
    } else {
        return;
    }
    m_session.selfRated();
    advance();
}

void SessionModel::grade(int value)
{
    if (!onItem() || !m_revealed || m_session.current().type != lq::ItemType::Card)
        return;
    const lq::Grade grade = value == 0 ? lq::Grade::Again : value == 3 ? lq::Grade::Easy : lq::Grade::Good;
    const lq::Item item = m_session.current();
    m_learner->apply([&](lq::LearnerState &state) {
        return lq::gradeReview(state, item.cardId, item.unit, grade, m_learner->day());
    });
    m_session.selfRated();
    if (grade == lq::Grade::Again)
        m_session.again();
    advance();
}

void SessionModel::next()
{
    if (!onItem() || !m_checked)
        return;
    advance();
}

void SessionModel::startSecondTry()
{
    if (!m_active || !m_intermission)
        return;
    m_intermission = false;
    startItem();
}

void SessionModel::skipSecondTry()
{
    if (!m_active || !m_intermission)
        return;
    m_intermission = false;
    m_session.skipSecondTry();
    finish();
}

// Moves on. The second-try round of checked answers waits behind a screen of its own, with what it is worth,
// until the learner starts it; review cards graded "again" just come back, as they earn no extra XP.
void SessionModel::advance()
{
    m_session.next();
    if (m_session.finished()) {
        finish();
    } else if (m_session.secondTry() && m_session.roundPosition() == 0
               && m_session.current().type != lq::ItemType::Card) {
        m_intermission = true;
        m_item.clear();
        m_checked = false;
        m_feedback.clear();
        emit changed();
    } else {
        startItem();
    }
}

// ---- Finishing -----------------------------------------------------------------------------------------

void SessionModel::finish()
{
    m_item.clear();
    const auto &activity = m_learner->state().activity;
    const auto today = activity.find(m_learner->day());
    const int xp = (today == activity.end() ? 0 : today->second.xp) - m_xpAtStart;

    QVariantList missed;
    for (const lq::Item &item : m_session.missed()) {
        if (missed.size() == 6)
            break;
        std::string answer = item.type == lq::ItemType::Choice ? lq::correctAnswer(item) : lq::sentenceOf(item);
        if (answer.empty() && !item.answers.empty())
            answer = item.answers.front();
        std::string question = item.question.empty() ? (item.en.empty() ? item.word : item.en) : item.question;
        if (item.type == lq::ItemType::Conjugate)
            question = item.verb + " · " + std::string(lq::tenseInfo(item.tense).en);
        missed.append(QVariantMap{{u"question"_s, qs(question)}, {u"answer"_s, qs(answer)}, {u"why"_s, qs(item.why)}});
    }

    const double score = m_session.score();
    m_result = {
        {u"score"_s, score},
        {u"percent"_s, static_cast<int>(std::lround(score * 100))},
        {u"answered"_s, m_session.answered()},
        {u"firstTry"_s, m_session.firstTry()},
        {u"selfRated"_s, m_session.selfRatedCount()},
        {u"xp"_s, xp},
        {u"missed"_s, missed},
        {u"secondTries"_s, m_session.secondTries()},
        {u"fixed"_s, m_session.fixed()},
        {u"empty"_s, m_session.size() == 0},
    };

    // Lessons and practice rounds count towards the next reflection; checkpoints and the mock exam are tests.
    if ((m_kind == "lesson"_L1 || m_kind == "trainer"_L1) && m_session.size() > 0) {
        m_learner->countSession();
        m_result[u"activity"_s] = m_kind == "lesson"_L1 ? u"lesson"_s : m_trainer;
        m_result[u"kinds"_s] = LearnerController::kindList(m_kinds);
        m_result[u"reflect"_s] = m_learner->reflectionDue();
    }

    if (m_kind == "trainer"_L1) {
        if (const auto task = lq::taskFromId(m_trainer.toStdString()))
            m_result[u"marked"_s] = m_learner->completeTask(*task);
    } else if (m_kind == "checkpoint"_L1) {
        const int phase = m_trainer.toInt();
        m_learner->apply([&](lq::LearnerState &state) { return lq::saveCheckpoint(state, phase, score, m_learner->day()); });
        m_result[u"passed"_s] = score >= kCheckpointPass;
        // The lessons behind the misses, most missed first.
        std::vector<std::pair<int, int>> weak;  // unit, misses
        for (const lq::Item &item : m_session.missed()) {
            if (item.unit < 0)
                continue;
            const auto it = std::find_if(weak.begin(), weak.end(), [&](const auto &w) { return w.first == item.unit; });
            if (it == weak.end())
                weak.emplace_back(item.unit, 1);
            else
                ++it->second;
        }
        std::stable_sort(weak.begin(), weak.end(), [](const auto &a, const auto &b) { return a.second > b.second; });
        QVariantList units;
        if (const lq::Course *course = m_learner->course()) {
            for (const auto &[unit, misses] : weak) {
                const lq::Unit &u = course->units.at(static_cast<std::size_t>(unit));
                units.append(QVariantMap{{u"index"_s, unit}, {u"n"_s, u.n}, {u"title"_s, qs(u.title)}, {u"misses"_s, misses}});
            }
        }
        m_result[u"weakUnits"_s] = units;
    } else if (m_kind == "mock"_L1 || m_kind == "assessment"_L1) {
        m_result[u"part"_s] = m_trainer;
    }
    if (m_kind == "assessment"_L1)
        m_result.remove(u"missed"_s);  // what was missed would give the answers away
    emit changed();
    emit completed();
}
