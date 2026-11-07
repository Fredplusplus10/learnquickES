/* App shell: startup, hash router, sidebar/titlebar, event delegation, ⌘K search and active-time tracking. */
(function (root) {
  'use strict';
  const LQ = root.LQ;
  const U = LQ.util, P = LQ.progress, C = LQ.content, V = LQ.views;
  const { ico, esc } = LQ.ui;
  const L = (en, es) => LQ.L(en, es);
  const $ = id => document.getElementById(id);

  const NAV = [['', 'home', ['Home', 'Inicio']], ['today', 'calendar', ["Today's plan", 'Plan de hoy']], ['lessons', 'book', ['Lessons', 'Lecciones']], ['practice', 'target', ['Practice', 'Práctica']], ['vocab', 'cards', ['Vocabulary', 'Vocabulario']], ['immersion', 'headphones', ['Immersion', 'Inmersión']], ['progress', 'chart', ['Progress', 'Progreso']]];
  const ROUTES = { '': 'dashboard', today: 'today', lessons: 'lessons', lesson: 'lesson', practice: 'practice', vocab: 'vocab', immersion: 'immersion', progress: 'progress', settings: 'settings', checkpoint: 'checkpoint', onboarding: 'onboarding' };
  const NAV_OF = { lesson: 'lessons', checkpoint: 'lessons' };
  let cleanup = null, lastRoute = null, day = null, lastInteraction = Date.now();

  const save = () => LQ.store.save(LQ.state);
  function parseHash() { const key = location.hash.replace(/^#\/?/, ''), [name, ...rest] = key.split('/'); return { key, name, param: rest.join('/') || null }; }
  function go(route) { const target = '#/' + route; LQ.ui.closeModal(); if (location.hash === target || (!location.hash && !route)) { lastRoute = null; render(); } else location.hash = target; }

  // Review is a no-op when nothing is due: tick it automatically, and untick it if a "get ahead" day was
  // marked earlier but new cards have come due since.
  function syncReview() {
    const S = LQ.state, today = U.today(), st = P.planStatus(S);
    if (!S.onboarded || st.finished || !st.tasks.includes('review')) return;
    const due = LQ.srs.queue(S.cards, today, 1).length > 0, mark = S.tasks[st.n] && S.tasks[st.n].review;
    if (mark && mark < today && due) { delete S.tasks[st.n].review; save(); }
    else if (!mark && !due) { handle(P.markTask(S, 'review', today)); save(); }
  }

  function render() {
    const S = LQ.state, r = parseHash(), main = $('mainContent');
    if (!S.onboarded && r.name !== 'onboarding') return go('onboarding');
    if (S.onboarded && r.name === 'onboarding') return go('');
    if (!(r.name in ROUTES)) return go('');
    const fresh = r.key !== lastRoute; lastRoute = r.key;
    if (fresh) $('searchResults').hidden = true;
    syncReview();
    // Keep the caret in the field being typed in (search boxes re-render on every keystroke).
    const active = document.activeElement, focusId = active && active.id && main.contains(active) ? active.id : null;
    let sel = null; try { sel = focusId ? [active.selectionStart, active.selectionEnd] : null; } catch { sel = null; }
    if (cleanup) { try { cleanup(); } catch (e) { console.error(e); } cleanup = null; }
    let out;
    try { out = V[ROUTES[r.name]](r.param, { fresh }); }
    catch (e) { console.error(e); out = LQ.ui.emptyState('bell', L('Something went wrong on this page', 'Algo falló en esta página'), String(e && e.message || e), `<button type="button" class="primary-btn" data-go="">${L('Go home', 'Ir al inicio')}</button>`); }
    main.innerHTML = typeof out === 'string' ? out : out.html;
    if (out && out.mount) cleanup = out.mount(main) || null;
    if (fresh) main.scrollTop = 0;
    if (focusId && $(focusId)) { const f = $(focusId); f.focus(); try { if (sel && sel[0] !== null) f.setSelectionRange(sel[0], sel[1]); } catch { /* not a text field */ } }
    chrome(r);
    LQ.ui.flushCelebrations();
  }

  // Sidebar + titlebar reflect the current language, route, streak and island state.
  function chrome(r = parseHash()) {
    const S = LQ.state, today = U.today(), onb = !S.onboarded, active = NAV_OF[r.name] ?? r.name;
    document.documentElement.lang = LQ.lang();
    document.body.classList.toggle('is-onboarding', onb);
    document.body.classList.toggle('is-island', !!LQ.island.active);
    $('sidebarNavigation').innerHTML = NAV.map(([route, icon, label]) => `<button type="button" class="nav-item ${active === route ? 'active' : ''}" data-go="${route}" ${active === route ? 'aria-current="page"' : ''}>${ico(icon)}<span>${esc(L(...label))}</span></button>`).join('');
    const settings = $('settingsButton'); settings.classList.toggle('active', r.name === 'settings'); settings.querySelector('span').textContent = L('Settings', 'Ajustes');
    if (!onb) {
      const st = P.planStatus(S), si = P.streakInfo(S, today), xp = (S.activity[today] || {}).xp || 0;
      $('sidebarNote').innerHTML = `<span class="mini-flag" aria-hidden="true"><img src="assets/icons/spanish-flag.svg" alt=""></span><div><b>${L('Spanish → B1', 'Español → B1')}</b><small>${st.finished ? L('Plan complete', 'Plan completado') : L(`Day ${st.n} of ${st.total}`, `Día ${st.n} de ${st.total}`)}</small></div>`;
      $('titleStatus').innerHTML = `${LQ.island.active ? `<button type="button" class="status-pill island" data-go="practice/island" title="Isla lingüística">${ico('palm')}<b data-island-clock>${LQ.island.clock()}</b></button>` : ''}
        <button type="button" class="status-pill ${si.todayDone ? 'lit' : ''}" data-go="progress" title="${L(`${si.count}-day streak · ${si.freezes} freezes`, `Racha de ${si.count} días · ${si.freezes} protectores`)}">${ico('flame')}<b>${si.count}</b>${si.freezes ? `<span class="freeze-count">${ico('shield')}${si.freezes}</span>` : ''}</button>
        <span class="status-pill xp" title="${L('XP today (10 keeps the streak)', 'XP hoy (10 mantienen la racha)')}">${ico('bolt')}<b>${xp}</b> XP</span>`;
    } else { $('sidebarNote').innerHTML = ''; $('titleStatus').innerHTML = ''; }
    $('searchInput').placeholder = L('Search lessons, words, trainers…', 'Busca lecciones, palabras, entrenadores…');
    const label = r.name === 'lesson' && C.unitById[r.param] ? C.unitById[r.param].title : (NAV.find(n => n[0] === active) || [0, 0, [r.name === 'settings' ? 'Settings' : 'LearnQuick']])[2][0];
    document.title = `${label} · LearnQuick`;
  }

  // Domain events → celebrations and toasts. Achievements are re-checked after anything that changed progress.
  function handle(events) {
    if (!events || !events.length) return;
    const S = LQ.state;
    for (const e of events) {
      if (e.type === 'streak' || e.type === 'dayComplete' || e.type === 'phaseComplete') LQ.ui.celebrate(e);
      else if (e.type === 'freeze') LQ.ui.toast(L('You earned a streak freeze! It protects one missed study day.', '¡Ganaste un protector de racha! Cubre un día de estudio perdido.'));
      else if (e.type === 'freezeUsed') LQ.ui.toast(L(`A streak freeze protected ${e.dates.length > 1 ? e.dates.length + ' missed days' : 'a missed day'}. Your streak is safe.`, `Un protector cubrió ${e.dates.length > 1 ? e.dates.length + ' días perdidos' : 'un día perdido'}. Tu racha está a salvo.`));
      else if (e.type === 'achievement') LQ.ui.toast(L(`🏅 Achievement unlocked: ${e.achievement.en}`, `🏅 Logro desbloqueado: ${e.achievement.es}`));
    }
    if (events.some(e => e.type !== 'achievement')) handle(P.checkAchievements(S, U.today()));
    chrome();
  }
  function replaceState(S) { LQ.state = P.migrate(S, U.today()); LQ.island.active = false; save(); lastRoute = null; }

  // ---- ⌘K search across lessons, words and trainers ----------------------------------------------------------
  let index = null, results = [], cursor = 0;
  function buildIndex() {
    const out = [];
    for (const u of C.units) out.push({ kind: 'lesson', label: `${u.n}. ${u.title}`, sub: u.es, route: 'lesson/' + u.id, text: `${u.title} ${u.es} ${u.canDo} ${u.grammar.title}` });
    for (const v of Object.values(C.vocabIndex)) out.push({ kind: 'word', label: v.es, sub: v.en, route: 'vocab/' + encodeURIComponent(v.es.replace(/^(el|la|los|las)\s+/i, '')), text: `${v.es} ${v.en}`, speak: v.es });
    const pages = [['practice/verbs', 'Verb gym · conjugation'], ['practice/past', 'Fui vs. iba · preterite imperfect'], ['practice/cognates', 'Cognates lab'], ['practice/falsefriends', 'False friends'], ['practice/gender', 'El / la gender trainer'], ['practice/speak', 'Speaking · shadowing · pronunciation'], ['practice/island', 'Language island · isla'], ['practice/describe', 'Describe it · circumlocution'], ['practice/review', 'Review flashcards'], ['immersion', 'Immersion · podcasts · devices · exchanges'], ['checkpoint/5', 'B1 mock exam · DELE'], ['settings', 'Settings · voice · backup · plan']];
    for (const [route, label] of pages) out.push({ kind: 'page', label, sub: '', route, text: label });
    return out.map(x => ({ ...x, key: U.stripAccents(U.normalize(x.text)) }));
  }
  function search(q) {
    index = index || buildIndex();
    const k = U.stripAccents(U.normalize(q)); if (!k) return [];
    const scored = index.filter(x => x.key.includes(k)).map(x => ({ x, s: (U.stripAccents(U.normalize(x.label)).startsWith(k) ? 0 : 1) + (x.kind === 'page' ? 0 : x.kind === 'lesson' ? 0.2 : 0.4) }));
    return scored.sort((a, b) => a.s - b.s).slice(0, 8).map(r => r.x);
  }
  function drawResults() {
    const box = $('searchResults'), q = $('searchInput').value;
    if (!q.trim()) { box.hidden = true; return; }
    const KIND = { lesson: L('Lesson', 'Lección'), word: L('Word', 'Palabra'), page: L('Page', 'Página') };
    box.innerHTML = results.length ? results.map((r, i) => `<button type="button" class="search-hit ${i === cursor ? 'on' : ''}" data-hit="${i}" role="option" aria-selected="${i === cursor}"><span class="hit-kind">${KIND[r.kind]}</span><b ${r.kind === 'word' ? 'lang="es"' : ''}>${esc(r.label)}</b><small>${esc(r.sub)}</small></button>`).join('') : `<p class="search-empty">${L('No results', 'Sin resultados')}</p>`;
    box.hidden = false;
  }
  function openHit(i) { const r = results[i]; if (!r) return; $('searchInput').value = ''; $('searchResults').hidden = true; $('searchInput').blur(); if (r.speak) LQ.speech.speak(r.speak); go(r.route); }
  function bindSearch() {
    const input = $('searchInput'), box = $('searchResults');
    input.addEventListener('input', () => { results = search(input.value); cursor = 0; drawResults(); });
    input.addEventListener('keydown', e => {
      if (e.key === 'ArrowDown' || e.key === 'ArrowUp') { e.preventDefault(); cursor = (cursor + (e.key === 'ArrowDown' ? 1 : -1) + results.length) % Math.max(1, results.length); drawResults(); }
      else if (e.key === 'Enter') { e.preventDefault(); openHit(cursor); }
      else if (e.key === 'Escape') { input.value = ''; box.hidden = true; input.blur(); }
    });
    input.addEventListener('blur', () => setTimeout(() => { box.hidden = true; }, 150));
    box.addEventListener('mousedown', e => { const b = e.target.closest('[data-hit]'); if (b) { e.preventDefault(); openHit(+b.dataset.hit); } });
    document.addEventListener('keydown', e => { if ((e.metaKey || e.ctrlKey) && e.key.toLowerCase() === 'k') { e.preventDefault(); input.focus(); input.select(); } });
  }

  // ---- Delegated events --------------------------------------------------------------------------------------
  Object.assign(LQ.actions = LQ.actions || {}, {
    'shadow-repeat': b => LQ.ui.shadowRepeat($(b.dataset.target)),
    'shadow-record': b => LQ.ui.shadowRecord($(b.dataset.target), b),
    'shadow-recognize': b => LQ.ui.shadowRecognize($(b.dataset.target))
  });
  function bindEvents() {
    document.addEventListener('click', e => {
      const el = e.target.closest('[data-go],[data-speak],[data-action],[data-accent]');
      if (!el || el.disabled) return;
      if (el.hasAttribute('data-speak')) LQ.speech.speak(el.dataset.speak, { rate: el.dataset.rate ? +el.dataset.rate : undefined });
      else if (el.hasAttribute('data-accent')) LQ.ui.insertAccent(el);
      else if (el.hasAttribute('data-action')) { const fn = LQ.actions[el.dataset.action]; if (fn) fn(el, e); }
      else { e.preventDefault(); go(el.dataset.go); }
    });
    document.addEventListener('change', e => { const el = e.target.closest('[data-change]'); if (el && LQ.changes[el.dataset.change]) LQ.changes[el.dataset.change](el, e); });
    document.addEventListener('input', e => { const el = e.target.closest('[data-input]'); if (el && LQ.inputs[el.dataset.input]) LQ.inputs[el.dataset.input](el, e); });
    document.addEventListener('submit', e => { const f = e.target.closest('[data-submit]'); if (f) { e.preventDefault(); if (LQ.submits[f.dataset.submit]) LQ.submits[f.dataset.submit](f, e); } });
    $('goBack').addEventListener('click', () => history.back());
    $('goForward').addEventListener('click', () => history.forward());
    window.addEventListener('hashchange', render);
    for (const ev of ['pointerdown', 'keydown', 'wheel', 'touchstart']) document.addEventListener(ev, () => { lastInteraction = Date.now(); }, { passive: true, capture: true });
    document.addEventListener('visibilitychange', () => { if (document.visibilityState === 'visible') tick(); });
  }

  // Active study time: counted in 15 s steps while the window is visible and used in the last 2 minutes.
  // Island time is logged separately when the session ends, so it is not double-counted here.
  function tick() {
    const S = LQ.state, today = U.today();
    if (today !== day) { day = today; if (S.onboarded) handle(P.applyFreezes(S, today)); save(); chrome(); }
  }
  function startTimers() {
    setInterval(() => {
      tick();
      const S = LQ.state;
      if (!S.onboarded || LQ.island.active || document.visibilityState !== 'visible' || Date.now() - lastInteraction > 120000) return;
      P.addSeconds(S, 15, U.today()); save();
    }, 15000);
    setInterval(() => { if (!LQ.island.active) return; for (const el of document.querySelectorAll('[data-island-clock]')) el.textContent = LQ.island.clock(); }, 1000);
  }

  function start() {
    document.documentElement.classList.add('native-shell');
    LQ.state = LQ.store.load(); day = U.today();
    const S = LQ.state;
    if (S.islandStart && Date.now() - S.islandStart < 3 * 3600 * 1000) LQ.island = { ...LQ.island, active: true, start: S.islandStart, prompt: 0 };
    else delete S.islandStart;
    const events = S.onboarded ? P.applyFreezes(S, day) : [];
    LQ.store.persist(); save();
    bindEvents(); bindSearch(); startTimers();
    render();
    handle(events);
  }

  LQ.app = { start, render, go, save, handle, replaceState, chrome };
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start); else start();
})(window);
