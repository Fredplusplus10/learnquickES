const test = require('node:test');
const assert = require('node:assert/strict');
const LQ = require('./load')();

function storage(initial = null, failWrites = false) {
  let raw = initial;
  globalThis.localStorage = {
    getItem() { return raw; },
    setItem(k, value) { if (failWrites) throw new Error('QuotaExceededError'); raw = value; },
  };
  return { get raw() { return raw; }, set raw(value) { raw = value; } };
}
const state = () => LQ.progress.defaultState('2026-11-02');

test('a damaged stored document is not overwritten on app startup', () => {
  const disk = storage('{bad-json');
  const loaded = LQ.store.load();
  assert.equal(loaded.version, 1);
  loaded.profile.name = 'Fallback only';
  assert.equal(LQ.store.save(loaded), false);
  assert.equal(disk.raw, '{bad-json');
  assert.equal(LQ.store.available(), false);
  assert.equal(LQ.store.replace(loaded), true, 'an explicit user restore may replace damaged data');
  assert.equal(JSON.parse(disk.raw).profile.name, 'Fallback only');
});

test('a second tab cannot silently overwrite progress changed elsewhere', () => {
  const disk = storage(JSON.stringify(state()));
  const loaded = LQ.store.load();
  const elsewhere = state();
  elsewhere.profile.name = 'Saved in other tab';
  disk.raw = JSON.stringify(elsewhere);
  loaded.profile.name = 'Stale tab';
  assert.equal(LQ.store.save(loaded), false);
  assert.equal(JSON.parse(disk.raw).profile.name, 'Saved in other tab');
});

test('a failed write preserves the last successfully saved snapshot', () => {
  const disk = storage(null);
  const s = LQ.store.load();
  assert.equal(LQ.store.save(s), true);
  const old = disk.raw;
  globalThis.localStorage.setItem = () => { throw new Error('QuotaExceededError'); };
  s.profile.name = 'Not persisted';
  assert.equal(LQ.store.save(s), false);
  assert.equal(disk.raw, old);
});
