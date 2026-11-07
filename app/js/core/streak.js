/* Streak rules (pure). A day counts once the learner earns MIN_XP. Non-study days (weekends, holidays, days off)
   never break the streak; studying on them still adds a day. A missed study day consumes a freeze if one is held. */
(function (root) {
  'use strict';
  const LQ = root.LQ = root.LQ || {};
  const U = LQ.util;
  const MIN_XP = 10, MAX_FREEZES = 2, FREEZE_EVERY = 5;

  // active/frozen: Set of ISO dates. isScheduled(iso) → bool. Today without activity yet does not break anything.
  function current(today, active, frozen, isScheduled) {
    if (!active.size) return 0;
    const first = [...active].sort()[0];
    let d = active.has(today) ? today : U.addDays(today, -1), count = 0;
    while (d >= first) {
      if (active.has(d)) count++;
      else if (!frozen.has(d) && isScheduled(d)) break;
      d = U.addDays(d, -1);
    }
    return count;
  }

  // Missed study days between the last active day and today: protect them with freezes while any remain.
  // Returns the dates that consumed a freeze; the caller persists them and the new freeze count.
  function freezesNeeded(today, active, frozen, isScheduled, freezes) {
    if (!active.size) return { used: [], freezes, broken: false };
    const last = [...active].sort().pop(), used = [];
    let left = freezes, broken = false;
    for (let d = U.addDays(last, 1); d < today; d = U.addDays(d, 1)) {
      if (active.has(d) || frozen.has(d) || !isScheduled(d)) continue;
      if (left > 0) { left--; used.push(d); } else { broken = true; break; }
    }
    return { used, freezes: left, broken };
  }

  // Freezes earned when today's activity first reaches MIN_XP: every 5th streak day, and any study on a day off.
  function earned(streakCount, todayScheduled, freezes) {
    let gain = 0;
    if (streakCount > 0 && streakCount % FREEZE_EVERY === 0) gain++;
    if (!todayScheduled) gain++;
    return Math.min(MAX_FREEZES, freezes + gain) - freezes;
  }

  // Longest run, using the same bridging rules (for the Progress screen).
  function longest(active, frozen, isScheduled) {
    if (!active.size) return 0;
    const days = [...active].sort(); let best = 0, run = 0;
    for (let d = days[0]; d <= days[days.length - 1]; d = U.addDays(d, 1)) {
      if (active.has(d)) { run++; best = Math.max(best, run); }
      else if (!frozen.has(d) && isScheduled(d)) run = 0;
    }
    return best;
  }

  LQ.streak = { MIN_XP, MAX_FREEZES, FREEZE_EVERY, current, freezesNeeded, earned, longest };
  if (typeof module !== 'undefined') module.exports = LQ;
})(typeof window !== 'undefined' ? window : globalThis);
