/* Learning domain: state shape, XP/streak events, daily plan, pace, totals and achievements.
   Functions take the state object S and return events for the UI; nothing here touches the DOM or storage. */
(function (root) {
  'use strict';
  const LQ = root.LQ = root.LQ || {};
  const U = LQ.util, C = LQ.calendar, K = LQ.streak, R = LQ.srs;
  const XP = { correct: 10, almost: 6, retry: 5, card: 2, phrase: 5, lessonBonus: 20, dayBonus: 20 };

  function defaultState(today) {
    return {
      version: 1, createdAt: today, onboarded: false,
      profile: { name: '', level: null, placement: null, accent: 'es-MX', voiceURI: '', rate: 0.9, uiLang: 'en', vosotros: false, reviewLimit: 20 },
      plan: { days: 60, start: today, studyDays: [1, 2, 3, 4, 5], skipHolidays: true, daysOff: [], reminder: '18:30' },
      done: [], tasks: {}, units: {}, checkpoints: {}, cards: {}, activity: {},
      streak: { freezes: 0, frozen: [], longest: 0 },
      immersion: [], exchanges: [], missions: {}, achievements: {}, writing: [], lastBackup: null
    };
  }
  // Fill fields added in later versions so old backups keep loading, and give every field the type the views
  // expect. A backup is a file anyone can edit: a number that arrives as text full of markup must not reach the
  // page as markup, so numbers become numbers, dates ISO dates and text text, and anything else is dropped.
  function migrate(raw, today) {
    raw = raw && typeof raw === 'object' && !Array.isArray(raw) ? raw : {};
    const base = defaultState(today), S = { ...base, ...raw };
    for (const k of ['profile', 'plan', 'streak']) S[k] = { ...base[k], ...(isObj(raw[k]) ? raw[k] : {}) };
    if (![30, 60, 90].includes(+S.plan.days)) S.plan.days = 60;
    S.plan.days = +S.plan.days;
    return sanitize(S, base);
  }

  const isObj = v => !!v && typeof v === 'object' && !Array.isArray(v);
  const ISO = /^\d{4}-\d{2}-\d{2}$/;
  const day = (v, fallback = null) => (typeof v === 'string' && ISO.test(v) ? v : fallback);
  const int = (v, lo = 0, hi = 1e9, fallback = 0) => (Number.isFinite(+v) ? Math.min(hi, Math.max(lo, Math.round(+v))) : fallback);
  const num = (v, fallback = 0) => (Number.isFinite(+v) ? +v : fallback);
  const text = (v, max = 140) => (typeof v === 'string' ? v.slice(0, max) : '');
  // Short labels and ids some views put in attributes: no character that could open or close markup.
  const token = (v, max = 40) => text(v, max).replace(/[<>&"'`]/g, '');
  const list = v => (Array.isArray(v) ? v : []);
  const map = (v, f) => Object.fromEntries(Object.entries(isObj(v) ? v : {}).map(([k, x]) => [k, f(x)]).filter(([, x]) => x !== undefined));
  const result = r => (isObj(r) ? { date: day(r.date), score: num(r.score), best: num(r.best), ...(r.attempts !== undefined ? { attempts: int(r.attempts, 1) } : {}) } : undefined);

  function sanitize(S, base) {
    const p = S.profile, b = base.profile;
    S.profile = {
      ...p, name: text(p.name, 30), level: typeof p.level === 'string' ? token(p.level, 20) : null,
      placement: p.placement === null || p.placement === undefined ? null : num(p.placement), accent: token(p.accent, 20) || b.accent,
      voiceURI: token(p.voiceURI, 200), rate: num(p.rate, b.rate), uiLang: p.uiLang === 'es' ? 'es' : 'en',
      vosotros: !!p.vosotros, reviewLimit: int(p.reviewLimit, 1, 500, b.reviewLimit)
    };
    S.plan = {
      ...S.plan, start: day(S.plan.start, base.plan.start), studyDays: list(S.plan.studyDays).map(d => int(d, 0, 6)),
      skipHolidays: S.plan.skipHolidays !== false, daysOff: list(S.plan.daysOff).map(d => day(d)).filter(Boolean),
      reminder: /^\d{2}:\d{2}$/.test(S.plan.reminder) ? S.plan.reminder : base.plan.reminder
    };
    S.streak = { freezes: int(S.streak.freezes, 0, 2), frozen: list(S.streak.frozen).map(d => day(d)).filter(Boolean), longest: int(S.streak.longest) };
    S.done = list(S.done).filter(isObj).map(d => ({ n: int(d.n, 1), date: day(d.date) })).filter(d => d.date);
    S.tasks = map(S.tasks, t => (isObj(t) ? map(t, d => day(d) || undefined) : undefined));
    S.units = map(S.units, result);
    S.checkpoints = map(S.checkpoints, result);
    S.cards = map(S.cards, c => (isObj(c) ? { box: int(c.box, 0, 7, 1), due: day(c.due, S.createdAt), reps: int(c.reps), lapses: int(c.lapses),
      added: day(c.added, S.createdAt), last: day(c.last) } : undefined));
    S.activity = map(S.activity, a => (isObj(a) ? { xp: int(a.xp), sec: int(a.sec), input: int(a.input), island: int(a.island), talk: int(a.talk),
      speak: int(a.speak), ex: map(a.ex, e => (Array.isArray(e) && e.length === 2 ? [int(e[0]), int(e[1])] : undefined)) } : undefined));
    for (const d of Object.keys(S.activity)) if (!ISO.test(d)) delete S.activity[d];
    const entry = e => isObj(e) && day(e.date) && int(e.min, 0, 600) > 0;
    S.immersion = list(S.immersion).filter(entry).map((e, i) => ({ id: token(e.id) || 'i' + i, date: e.date, min: int(e.min, 1, 600), kind: token(e.kind, 20),
      source: text(e.source), ...(e.imported === true ? { imported: true } : {}) }));
    S.exchanges = list(S.exchanges).filter(entry).map((e, i) => ({ id: token(e.id) || 'x' + i, date: e.date, min: int(e.min, 1, 600),
      partner: text(e.partner), note: text(e.note) }));
    S.missions = map(S.missions, d => (d === true ? true : day(d) || undefined));
    S.achievements = map(S.achievements, d => day(d) || undefined);
    S.writing = list(S.writing).filter(isObj).map(w => ({ date: day(w.date), words: int(w.words), text: text(w.text, 4000) })).filter(w => w.date);
    S.createdAt = day(S.createdAt, base.createdAt);
    S.lastBackup = day(S.lastBackup);
    S.onboarded = !!S.onboarded;
    return S;
  }

  const scheduleCache = {};
  const schedule = days => scheduleCache[days] || (scheduleCache[days] = C.buildSchedule(days));
  const scheduled = S => d => C.isStudyDay(d, S.plan);
  const act = (S, d) => S.activity[d] || (S.activity[d] = { xp: 0, sec: 0, input: 0, island: 0, talk: 0, speak: 0, ex: {} });
  const activeSet = S => new Set(Object.keys(S.activity).filter(d => S.activity[d].xp >= K.MIN_XP));
  const streakCount = (S, today) => K.current(today, activeSet(S), new Set(S.streak.frozen), scheduled(S));

  // XP drives the streak: crossing MIN_XP for the first time today extends it (Duolingo-style, separate from the plan).
  function addXP(S, n, today) {
    const a = act(S, today), before = a.xp, events = [];
    a.xp += n;
    if (before < K.MIN_XP && a.xp >= K.MIN_XP) {
      const count = streakCount(S, today);
      const gain = K.earned(count, C.isStudyDay(today, S.plan), S.streak.freezes);
      if (gain) { S.streak.freezes += gain; events.push({ type: 'freeze', gain }); }
      S.streak.longest = Math.max(S.streak.longest || 0, count);
      events.push({ type: 'streak', count });
    }
    return events;
  }
  function recordAnswer(S, skill, result, today) {
    const a = act(S, today), e = a.ex[skill] || (a.ex[skill] = [0, 0]);
    // Accuracy measures first attempts. Requeued corrections still award practice XP.
    if (!result.retry) { e[1]++; if (result.ok) e[0]++; }
    return addXP(S, result.ok ? (result.almost ? XP.almost : result.retry ? XP.retry : XP.correct) : 0, today);
  }
  const addSeconds = (S, sec, today) => { act(S, today).sec += sec; };
  function logMinutes(S, field, min, today) { act(S, today)[field] += min; return addXP(S, Math.round(min), today); }

  // Freezes are applied when the app opens: missed study days since the last active day.
  function applyFreezes(S, today) {
    const r = K.freezesNeeded(today, activeSet(S), new Set(S.streak.frozen), scheduled(S), S.streak.freezes);
    if (!r.used.length) return [];
    S.streak.frozen.push(...r.used); S.streak.freezes = r.freezes;
    return [{ type: 'freezeUsed', dates: r.used }];
  }
  function streakInfo(S, today) {
    const active = activeSet(S), frozen = new Set(S.streak.frozen), isSched = scheduled(S), mon = U.weekStart(today);
    const week = Array.from({ length: 7 }, (_, i) => {
      const d = U.addDays(mon, i);
      const state = active.has(d) ? 'done' : frozen.has(d) ? 'frozen' : d > today ? 'future' : d === today ? 'today' : isSched(d) ? 'missed' : 'off';
      return { date: d, state, scheduled: isSched(d) };
    });
    const count = K.current(today, active, frozen, isSched);
    return { count, freezes: S.streak.freezes, todayDone: active.has(today), week, longest: Math.max(S.streak.longest || 0, K.longest(active, frozen, isSched)), active, frozen };
  }

  // ---- Daily plan -------------------------------------------------------------------------------------
  function planStatus(S) {
    const total = S.plan.days, doneCount = S.done.length, finished = doneCount >= total;
    const n = Math.min(doneCount + 1, total), entry = schedule(total)[n - 1];
    const tasks = C.dayTasks(entry, total);
    return { n, entry, tasks, total, doneCount, finished, minutes: C.dayMinutes(tasks), unit: entry.type === 'lesson' ? LQ.content.units[entry.unit] : null };
  }
  function taskDone(S, n, task) {
    const entry = schedule(S.plan.days)[n - 1];
    if (task === 'lesson') return !!S.units[LQ.content.units[entry.unit].id];
    if (task === 'checkpoint' || task === 'final') return !!S.checkpoints[entry.phase];
    return !!(S.tasks[n] && S.tasks[n][task]);
  }
  function markTask(S, task, today) {
    const st = planStatus(S);
    if (st.finished || !st.tasks.includes(task)) return [];
    (S.tasks[st.n] || (S.tasks[st.n] = {}))[task] = today;
    return refreshDay(S, today);
  }
  // Completes the current plan day once every task is done; returns dayComplete / phaseComplete events.
  function refreshDay(S, today) {
    const st = planStatus(S);
    if (st.finished || !st.tasks.every(t => taskDone(S, st.n, t))) return [];
    S.done.push({ n: st.n, date: today });
    const events = [{ type: 'dayComplete', n: st.n }, ...addXP(S, XP.dayBonus, today)];
    if (st.entry.type === 'checkpoint') events.push({ type: 'phaseComplete', phase: st.entry.phase });
    return events;
  }

  function learnUnit(S, unit, score, today) {
    const prev = S.units[unit.id];
    S.units[unit.id] = { date: prev ? prev.date : today, score, best: Math.max(score, prev ? prev.best : 0), attempts: (prev ? prev.attempts : 0) + 1 };
    for (const v of unit.vocab) if (!S.cards[v.id]) S.cards[v.id] = { ...R.create(today), u: unit.id };
    return [...addXP(S, XP.lessonBonus, today), ...refreshDay(S, today)];
  }
  function saveCheckpoint(S, phase, score, today) {
    const prev = S.checkpoints[phase];
    S.checkpoints[phase] = { date: prev ? prev.date : today, score, best: Math.max(score, prev ? prev.best : 0) };
    return refreshDay(S, today);
  }

  // Compares completed plan days with the study days elapsed since the start date.
  function pace(S, today) {
    const total = S.plan.days, doneCount = S.done.length, yesterday = U.addDays(today, -1);
    const expected = Math.min(total, S.plan.start <= yesterday ? C.countStudyDays(S.plan.start, yesterday, S.plan) : 0);
    const remaining = total - doneCount, doneToday = S.done.some(d => d.date === today);
    const from = C.isStudyDay(today, S.plan) && !doneToday ? today : U.addDays(today, 1);
    return { doneCount, expected, diff: doneCount - expected, remaining, target: remaining > 0 ? C.nthStudyDay(from, remaining, S.plan) : (S.done[S.done.length - 1] || {}).date };
  }

  function totals(S, today) {
    const T = { xp: 0, appMin: 0, inputMin: 0, islandMin: 0, talkMin: 0, speakCount: 0, ex: {} };
    for (const a of Object.values(S.activity)) {
      T.xp += a.xp; T.appMin += a.sec / 60; T.inputMin += a.input; T.islandMin += a.island; T.talkMin += a.talk || 0; T.speakCount += a.speak || 0;
      for (const [k, [c, t]] of Object.entries(a.ex)) { const e = T.ex[k] || (T.ex[k] = [0, 0]); e[0] += c; e[1] += t; }
    }
    const cards = Object.values(S.cards);
    T.words = { seen: cards.length, remembered: cards.filter(c => c.box >= R.REMEMBERED_BOX).length, due: cards.filter(c => c.due <= today).length };
    T.spanishMin = T.appMin + T.inputMin + T.islandMin + T.talkMin;
    T.accuracy = k => { const e = T.ex[k]; return e && e[1] ? e[0] / e[1] : null; };
    return T;
  }

  // ---- Achievements: every badge is earned from recorded data, never shown as a fake sample. -----------
  const ACHIEVEMENTS = [
    { id: 'first_lesson', icon: 'book', en: 'First Lesson', es: 'Primera lección', test: S => Object.keys(S.units).length >= 1 },
    { id: 'streak_5', icon: 'flame', en: '5-Day Streak', es: 'Racha de 5', test: (S, T, st) => st.longest >= 5 },
    { id: 'streak_15', icon: 'flame', en: '15-Day Streak', es: 'Racha de 15', test: (S, T, st) => st.longest >= 15 },
    { id: 'streak_30', icon: 'flame', en: '30-Day Streak', es: 'Racha de 30', test: (S, T, st) => st.longest >= 30 },
    { id: 'verbs_100', icon: 'bolt', en: '100 Verb Forms', es: '100 conjugaciones', test: (S, T) => (T.ex.verbs || [0])[0] >= 100 },
    { id: 'past_master', icon: 'clock', en: 'Fui vs. Iba', es: 'Fui vs. iba', test: (S, T) => { const e = T.ex.past || [0, 0]; return e[1] >= 30 && e[0] / e[1] >= 0.85; } },
    { id: 'cognates_50', icon: 'sparkles', en: 'Cognate Hunter', es: 'Cazador de cognados', test: (S, T) => (T.ex.cognates || [0])[0] + (T.ex.falsefriends || [0])[0] >= 50 },
    { id: 'words_100', icon: 'cards', en: '100 Words Remembered', es: '100 palabras recordadas', test: (S, T) => T.words.remembered >= 100 },
    { id: 'input_10h', icon: 'headphones', en: '10 h Listening', es: '10 h de escucha', test: (S, T) => T.inputMin >= 600 },
    { id: 'input_50h', icon: 'headphones', en: '50 h Listening', es: '50 h de escucha', test: (S, T) => T.inputMin >= 3000 },
    { id: 'island_5h', icon: 'palm', en: 'Island Dweller', es: 'Isla lingüística', test: (S, T) => T.islandMin >= 300 },
    { id: 'devices', icon: 'globe', en: 'Goodbye English', es: 'Adiós al inglés', test: S => Object.keys(S.missions).length >= 5 },
    { id: 'exchange_1', icon: 'users', en: 'First Exchange', es: 'Primer intercambio', test: S => S.exchanges.length >= 1 },
    { id: 'phase_1', icon: 'route', en: 'Foundations Done', es: 'Fundamentos', test: S => !!S.checkpoints[1] },
    { id: 'phase_3', icon: 'route', en: 'Past Tense Unlocked', es: 'El pasado, dominado', test: S => !!S.checkpoints[3] },
    { id: 'b1_mock', icon: 'award', en: 'B1 Mock Passed', es: 'Simulacro B1 aprobado', test: S => (S.checkpoints[5] || {}).best >= 0.7 }
  ];
  function checkAchievements(S, today) {
    const T = totals(S, today), st = streakInfo(S, today), fresh = [];
    for (const a of ACHIEVEMENTS) if (!S.achievements[a.id] && a.test(S, T, st)) { S.achievements[a.id] = today; fresh.push(a); }
    return fresh.map(a => ({ type: 'achievement', achievement: a }));
  }

  // The level of the series to recommend (content/practice.js C.series): the placement level (A2+ counts as A2),
  // raised by passing the phase 2 checkpoint (A2) and the phase 4 one (B1). Same as lq::seriesLevel.
  function seriesLevel(S) {
    const passed = phase => ((S.checkpoints[phase] || {}).best || 0) >= 0.7;
    if (passed(4)) return 'B1';
    if (/^A2/.test(S.profile.level || '') || passed(2)) return 'A2';
    return 'A1';
  }

  LQ.progress = { XP, seriesLevel, defaultState, migrate, schedule, act, addXP, recordAnswer, addSeconds, logMinutes, applyFreezes, streakInfo, streakCount, planStatus, taskDone, markTask, refreshDay, learnUnit, saveCheckpoint, pace, totals, ACHIEVEMENTS, checkAchievements };
  if (typeof module !== 'undefined') module.exports = LQ;
})(typeof window !== 'undefined' ? window : globalThis);
