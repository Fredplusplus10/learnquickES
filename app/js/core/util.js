/* LearnQuick core utilities. Plain script (no modules) so the app opens from file:// and the same
   code runs under Node for the tests. Everything attaches to the global LQ namespace. */
(function (root) {
  'use strict';
  const LQ = root.LQ = root.LQ || {};

  const pad = n => String(n).padStart(2, '0');
  // Dates are local calendar days as 'YYYY-MM-DD' strings: comparable with < and immune to UTC shifts.
  const iso = d => `${d.getFullYear()}-${pad(d.getMonth() + 1)}-${pad(d.getDate())}`;
  const parse = s => { const [y, m, d] = s.split('-').map(Number); return new Date(y, m - 1, d); };
  const addDays = (s, n) => { const d = parse(s); d.setDate(d.getDate() + n); return iso(d); };
  // Math.round absorbs the 23/25-hour days around daylight-saving changes.
  const diffDays = (a, b) => Math.round((parse(b) - parse(a)) / 86400000);
  const weekday = s => parse(s).getDay();
  const util = {
    clock: () => new Date(),
    today() { return iso(util.clock()); },
    iso, parse, addDays, diffDays, weekday,
    weekStart(s) { const wd = weekday(s); return addDays(s, wd === 0 ? -6 : 1 - wd); }, // Monday
    fmtDate(s, lang, opts) { return parse(s).toLocaleDateString(lang === 'es' ? 'es-MX' : 'en-US', opts || { month: 'short', day: 'numeric', year: 'numeric' }); },

    esc: v => String(v ?? '').replace(/[&<>"']/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c])),
    clamp: (v, lo, hi) => Math.min(hi, Math.max(lo, v)),
    shuffle(list, rnd = Math.random) { const a = list.slice(); for (let i = a.length - 1; i > 0; i--) { const j = Math.floor(rnd() * (i + 1)); [a[i], a[j]] = [a[j], a[i]]; } return a; },
    sample(list, n, rnd) { return util.shuffle(list, rnd).slice(0, n); },
    // Deterministic generator so "today's" picks stay stable across reloads.
    seeded(seed) { let h = 2166136261; for (const c of String(seed)) h = Math.imul(h ^ c.charCodeAt(0), 16777619); return () => { h += 0x6D2B79F5; let t = h; t = Math.imul(t ^ (t >>> 15), t | 1); t ^= t + Math.imul(t ^ (t >>> 7), t | 61); return ((t ^ (t >>> 14)) >>> 0) / 4294967296; }; },
    uid: () => Date.now().toString(36) + Math.random().toString(36).slice(2, 7),

    // Answer comparison: punctuation and case never matter; accents are reported as "almost".
    normalize: s => String(s ?? '').toLowerCase().replace(/[¿?¡!.,;:"“”«»()]/g, ' ').replace(/\s+/g, ' ').trim(),
    stripAccents: s => s.normalize('NFD').replace(/[̀-ͯ]/g, ''),
    check(input, answers) {
      const list = [].concat(answers), got = util.normalize(input);
      if (!got) return { ok: false, empty: true, expected: list[0] };
      for (const a of list) if (util.normalize(a) === got) return { ok: true, exact: true, expected: a };
      for (const a of list) if (util.stripAccents(util.normalize(a)) === util.stripAccents(got)) return { ok: true, almost: true, expected: a };
      return { ok: false, expected: list[0] };
    },
    words: s => util.normalize(s).split(' ').filter(Boolean),
    // Article-aware gender: 'el café' → m, 'la mesa' → f, 'cansado / cansada' → null.
    gender(es) { const m = /^(el|la|los|las)\s/i.exec(es); return m ? (/^(el|los)$/i.test(m[1]) ? 'm' : 'f') : null; },
    slug: s => util.stripAccents(String(s).toLowerCase()).replace(/[^a-z0-9ñ]+/g, '-').replace(/^-|-$/g, '')
  };
  LQ.util = util;
  if (typeof module !== 'undefined') module.exports = LQ;
})(typeof window !== 'undefined' ? window : globalThis);
