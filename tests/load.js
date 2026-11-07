// Loads the browser scripts into Node in the same order as index.html (they attach to globalThis.LQ).
const path = require('path');
const APP = path.join(__dirname, '..', 'app', 'js');
const FILES = ['core/util', 'core/calendar', 'core/srs', 'core/streak', 'core/verbs', 'core/progress', 'content/units', 'content/practice', 'core/store', 'ui/components', 'ui/exercises'];
module.exports = function load() {
  delete globalThis.LQ;
  for (const f of FILES) { const p = path.join(APP, f + '.js'); delete require.cache[require.resolve(p)]; require(p); }
  const LQ = globalThis.LQ;
  LQ.state = LQ.progress.defaultState('2026-11-02');
  return LQ;
};
