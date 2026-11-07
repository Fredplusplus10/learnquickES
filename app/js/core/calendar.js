/* Study calendar: business days, US federal holidays and the 30/60/90-day course schedule. Pure functions. */
(function (root) {
  'use strict';
  const LQ = root.LQ = root.LQ || {};
  const U = LQ.util;
  const PHASES = 5, UNITS_PER_PHASE = 5;

  // n-th weekday of a month (n = -1 → last). month is 0-based, weekday 0 = Sunday.
  function nthWeekday(year, month, wd, n) {
    if (n > 0) { const first = new Date(year, month, 1).getDay(); return U.iso(new Date(year, month, 1 + ((wd - first + 7) % 7) + (n - 1) * 7)); }
    const lastDay = new Date(year, month + 1, 0); return U.iso(new Date(year, month, lastDay.getDate() - ((lastDay.getDay() - wd + 7) % 7)));
  }
  // Fixed-date holidays move to Friday/Monday when they fall on a weekend (federal "observed" rule).
  function observed(s) { const wd = U.weekday(s); return wd === 6 ? U.addDays(s, -1) : wd === 0 ? U.addDays(s, 1) : s; }
  const cache = {};
  function usHolidays(year) {
    if (cache[year]) return cache[year];
    const f = (m, d) => observed(U.iso(new Date(year, m, d)));
    return (cache[year] = new Set([
      f(0, 1), nthWeekday(year, 0, 1, 3), nthWeekday(year, 1, 1, 3), nthWeekday(year, 4, 1, -1), f(5, 19), f(6, 4),
      nthWeekday(year, 8, 1, 1), nthWeekday(year, 9, 1, 2), f(10, 11), nthWeekday(year, 10, 4, 4), f(11, 25)
    ]));
  }
  function isHoliday(s) { const y = +s.slice(0, 4); return usHolidays(y).has(s) || usHolidays(y + 1).has(s); }

  // plan: { studyDays:[1..5], skipHolidays:bool, daysOff:[iso] }
  function isStudyDay(s, plan) {
    if (!(plan.studyDays || [1, 2, 3, 4, 5]).includes(U.weekday(s))) return false;
    if (plan.skipHolidays !== false && isHoliday(s)) return false;
    return !(plan.daysOff || []).includes(s);
  }
  function countStudyDays(from, to, plan) { let n = 0; for (let d = from; d <= to; d = U.addDays(d, 1)) if (isStudyDay(d, plan)) n++; return n; }
  // Date of the k-th study day counting from `from` (inclusive). k <= 0 returns null.
  function nthStudyDay(from, k, plan) {
    if (k <= 0) return null;
    let d = from, guard = 0;
    for (;;) { if (isStudyDay(d, plan) && --k === 0) return d; d = U.addDays(d, 1); if (++guard > 3660) return null; }
  }

  // Same 25 units for every plan; longer plans interleave practice days. The last day of each phase is a checkpoint.
  function buildSchedule(total) {
    const per = total / PHASES, out = [];
    for (let p = 0; p < PHASES; p++) {
      const lessonAt = Array.from({ length: UNITS_PER_PHASE }, (_, i) => Math.floor(i * (per - 1) / UNITS_PER_PHASE));
      let practiceIdx = 0;
      for (let j = 0; j < per; j++) {
        const base = { n: p * per + j + 1, phase: p + 1 };
        const li = lessonAt.indexOf(j);
        if (j === per - 1) out.push({ ...base, type: 'checkpoint' });
        else if (li >= 0) out.push({ ...base, type: 'lesson', unit: p * UNITS_PER_PHASE + li });
        else out.push({ ...base, type: 'practice', idx: practiceIdx++ });
      }
    }
    return out;
  }

  // Focus trainers rotate per phase so practice days stay varied but follow the course.
  const FOCUS = { 1: ['cognates', 'gender', 'speak'], 2: ['speak', 'gender', 'cognates'], 3: ['past', 'speak', 'past'], 4: ['past', 'speak', 'falsefriends'], 5: ['past', 'speak', 'describe'] };
  const MINUTES = { review: 5, lesson: 20, verbs: 5, mixed: 8, speak: 6, cognates: 6, gender: 5, past: 8, falsefriends: 5, describe: 8, checkpoint: 15, final: 45 };
  function dayTasks(entry, totalDays) {
    if (entry.type === 'checkpoint') return ['review', entry.phase === PHASES ? 'final' : 'checkpoint'];
    if (entry.type === 'practice') return ['review', 'mixed', 'verbs', FOCUS[entry.phase][entry.idx % 3]];
    // The 30-day plan has no practice days, so each lesson day carries one focus trainer too.
    const tasks = ['review', 'lesson', 'verbs'];
    if (totalDays === 30) tasks.push(FOCUS[entry.phase][entry.unit % 3]);
    return tasks;
  }
  const dayMinutes = tasks => tasks.reduce((sum, t) => sum + (MINUTES[t] || 5), 0);
  // Daily immersion target (podcasts, series, island, exchanges) — the volume that the app alone cannot provide.
  const INPUT_TARGET = { 30: 90, 60: 60, 90: 45 };

  LQ.calendar = { usHolidays, isHoliday, isStudyDay, countStudyDays, nthStudyDay, buildSchedule, dayTasks, dayMinutes, nthWeekday, INPUT_TARGET, MINUTES, PHASES, UNITS_PER_PHASE };
  if (typeof module !== 'undefined') module.exports = LQ;
})(typeof window !== 'undefined' ? window : globalThis);
