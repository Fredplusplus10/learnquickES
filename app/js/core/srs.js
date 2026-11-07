/* Spaced repetition: a transparent Leitner-style scheduler. For a 30–90 day course a learner sees each card a
   handful of times, too few reviews to fit FSRS parameters, so fixed expanding intervals are the honest choice. */
(function (root) {
  'use strict';
  const LQ = root.LQ = root.LQ || {};
  const U = LQ.util;
  const INTERVALS = [0, 1, 2, 4, 8, 16, 32, 60]; // days, indexed by box
  const MAX_BOX = INTERVALS.length - 1;
  const REMEMBERED_BOX = 4; // recalled after 1, 2 and 4 days → next gap is 8 days

  const srs = {
    INTERVALS, REMEMBERED_BOX,
    create: today => ({ box: 1, due: U.addDays(today, 1), reps: 0, lapses: 0, added: today, last: null }),
    // grade: 0 = again, 2 = good, 3 = easy
    grade(card, g, today) {
      const c = { ...card, reps: card.reps + 1, last: today };
      if (g === 0) { c.box = 1; c.lapses++; c.due = U.addDays(today, 1); return c; }
      c.box = Math.min(MAX_BOX, card.box + (g === 3 ? 2 : 1));
      c.due = U.addDays(today, INTERVALS[c.box]);
      return c;
    },
    isDue: (card, today) => card.due <= today,
    status: card => card.box >= REMEMBERED_BOX ? 'remembered' : 'learning',
    // Due cards, most overdue first; ties broken by weakest box.
    queue(cards, today, limit = 20) {
      return Object.entries(cards).filter(([, c]) => c.due <= today)
        .sort(([, a], [, b]) => (a.due < b.due ? -1 : a.due > b.due ? 1 : a.box - b.box)).slice(0, limit).map(([id]) => id);
    }
  };
  LQ.srs = srs;
  if (typeof module !== 'undefined') module.exports = LQ;
})(typeof window !== 'undefined' ? window : globalThis);
