/* Local persistence. One JSON document in localStorage (no server, no telemetry), plus export/import so the
   learner can keep a backup file. "?demo" uses a separate key filled with sample data for previews. */
(function (root) {
  'use strict';
  const LQ = root.LQ;
  const U = LQ.util, P = LQ.progress, C = LQ.calendar;
  const demo = typeof location !== 'undefined' && new URLSearchParams(location.search).has('demo');
  const KEY = demo ? 'learnquick.demo.v1' : 'learnquick.v1';
  let available = true;

  function load() {
    const today = U.today();
    try {
      const raw = localStorage.getItem(KEY);
      if (raw) return P.migrate(JSON.parse(raw), today);
    } catch (e) { available = false; console.warn('LearnQuick: storage unavailable', e); }
    return demo ? demoState(today) : P.defaultState(today);
  }
  function save(S) {
    try { localStorage.setItem(KEY, JSON.stringify(S)); available = true; }
    catch (e) { available = false; console.warn('LearnQuick: could not save', e); }
  }
  // Ask the browser not to evict our data under storage pressure (no-op where unsupported).
  function persist() { try { navigator.storage && navigator.storage.persist && navigator.storage.persist(); } catch { /* optional */ } }

  function download(name, text, type) {
    const url = URL.createObjectURL(new Blob([text], { type }));
    const a = Object.assign(document.createElement('a'), { href: url, download: name });
    document.body.appendChild(a); a.click(); a.remove();
    setTimeout(() => URL.revokeObjectURL(url), 2000);
  }
  function exportBackup(S) {
    S.lastBackup = U.today(); save(S);
    download(`learnquick-backup-${U.today()}.json`, JSON.stringify({ app: 'LearnQuick', exported: new Date().toISOString(), state: S }, null, 1), 'application/json');
  }
  // Twice the most a browser's local storage can hold once exported (about 8 MB); a real backup is tens of KB.
  const MAX_BACKUP_BYTES = 16 * 1024 * 1024;
  function parseBackup(text) {
    if (typeof text !== 'string' || text.length > MAX_BACKUP_BYTES) throw new Error('This file is too large to be a LearnQuick backup.');
    const data = JSON.parse(text), S = data && (data.state || data);
    if (!S || typeof S !== 'object' || !S.profile || !S.plan || !S.activity) throw new Error('This file is not a LearnQuick backup.');
    return P.migrate(S, U.today());
  }

  // Sample learner ~3 weeks into a 60-day plan. Built through the real domain functions so it stays consistent.
  function demoState(today) {
    const S = P.defaultState(today), rnd = U.seeded('demo');
    Object.assign(S.profile, { name: 'Cory', level: 'A2', placement: 9 });
    S.onboarded = true;
    let start = today, back = 0;
    while (back < 16) { start = U.addDays(start, -1); if (C.isStudyDay(start, S.plan)) back++; }
    S.plan.start = start;
    // One missed study day, covered by a streak freeze, so the demo shows how freezes look.
    const skipped = C.nthStudyDay(start, 7, S.plan);
    S.streak.frozen.push(skipped);
    for (let d = start; d < today; d = U.addDays(d, 1)) {
      const study = C.isStudyDay(d, S.plan);
      if ((!study && rnd() > 0.35) || d === skipped) continue;
      const a = P.act(S, d);
      a.sec = 1200 + Math.round(rnd() * 1500); a.input = 20 + Math.round(rnd() * 50); a.island = rnd() > 0.6 ? 30 : 0; a.speak = 6 + Math.round(rnd() * 6);
      for (const [k, n] of [['vocab', 8], ['grammar', 6], ['listening', 3], ['verbs', 10], ['past', 4]]) { const t = Math.round(n * (0.6 + rnd())); a.ex[k] = [Math.round(t * (0.7 + rnd() * 0.25)), t]; }
      if (a.input) S.immersion.push({ id: U.uid(), date: d, min: a.input, kind: 'podcast', source: rnd() > 0.5 ? 'Dreaming Spanish' : 'No Hay Tos' });
      P.addXP(S, 60 + Math.round(rnd() * 90), d);
      if (study) {
        const st = P.planStatus(S);
        if (st.entry.type === 'checkpoint') P.saveCheckpoint(S, st.entry.phase, 0.87, d);
        for (const t of st.tasks) if (t === 'lesson') P.learnUnit(S, st.unit, 0.8 + rnd() * 0.2, d); else P.markTask(S, t, d);
      }
    }
    // Age the cards so the review queue looks realistic.
    for (const c of Object.values(S.cards)) { const age = U.diffDays(c.added, today); c.box = Math.min(5, 1 + Math.floor(age / 4)); c.reps = c.box; c.due = U.addDays(today, Math.floor(rnd() * 5) - 1); }
    Object.assign(S.missions, { phone: start, siri: start, keyboard: U.addDays(start, 2) });
    S.exchanges.push({ id: U.uid(), date: U.addDays(today, -3), min: 45, partner: 'Mariana (Tandem)', note: 'Hablamos de comida mexicana.' });
    P.checkAchievements(S, today);
    return S;
  }

  LQ.store = { KEY, MAX_BACKUP_BYTES, demo, load, save, persist, exportBackup, parseBackup, download, available: () => available, demoState };
})(typeof window !== 'undefined' ? window : globalThis);
