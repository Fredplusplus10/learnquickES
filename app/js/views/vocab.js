/* Vocabulary: every course word grouped by unit, with article colours, audio, example and review status. */
(function (root) {
  'use strict';
  const LQ = root.LQ;
  const U = LQ.util, P = LQ.progress, C = LQ.content, R = LQ.srs;
  const { ico, esc, speakBtn, wordHTML } = LQ.ui;
  const L = (en, es) => LQ.L(en, es);
  const V = LQ.views = LQ.views || {};
  LQ.actions = LQ.actions || {}; LQ.changes = LQ.changes || {}; LQ.inputs = LQ.inputs || {};
  const vs = { q: '', phase: 'all', status: 'all' };

  function status(S, id, today) {
    const c = S.cards[id];
    if (!c) return 'new';
    if (c.due <= today) return 'due';
    return R.status(c);
  }
  const LABEL = { new: ['Not studied', 'Sin estudiar'], learning: ['Learning', 'Aprendiendo'], remembered: ['Remembered', 'Recordada'], due: ['Due today', 'Para hoy'] };

  V.vocab = (param, ctx) => {
    if (ctx && ctx.fresh) vs.q = param ? decodeURIComponent(param) : '';
    const S = LQ.state, today = U.today(), T = P.totals(S, today), q = U.stripAccents(U.normalize(vs.q));
    const match = v => !q || U.stripAccents(U.normalize(v.es + ' ' + v.en)).includes(q);
    const groups = C.units.filter(u => vs.phase === 'all' || u.phase === +vs.phase)
      .map(u => ({ u, words: u.vocab.filter(v => match(v) && (vs.status === 'all' || status(S, v.id, today) === vs.status)) }))
      .filter(g => g.words.length);
    const count = groups.reduce((n, g) => n + g.words.length, 0);
    return `<section class="page-head"><div class="card-head"><div><h1>${L('Vocabulary', 'Vocabulario')}</h1>
      <p class="sub">${L(`${T.words.seen} words in your deck · ${T.words.remembered} remembered · ${T.words.due} due today. Words join your deck when you finish their lesson.`, `${T.words.seen} palabras en tu mazo · ${T.words.remembered} recordadas · ${T.words.due} para hoy. Las palabras entran al mazo al terminar su lección.`)}</p></div>
      <button type="button" class="primary-btn" data-go="practice/review" ${T.words.due ? '' : 'disabled'}>${ico('refresh', 'white')}${L(`Review ${T.words.due} due`, `Repasar ${T.words.due}`)}</button></div></section>
    <section class="card filter-bar">
      <label class="search-field">${ico('search')}<input id="vocabSearch" type="search" value="${esc(vs.q)}" data-input="vocab-q" placeholder="${L('Search Spanish or English…', 'Busca en español o inglés…')}" aria-label="${L('Search words', 'Buscar palabras')}"></label>
      <select data-change="vocab-phase" aria-label="${L('Phase', 'Fase')}"><option value="all">${L('All phases', 'Todas las fases')}</option>${C.phases.map(p => `<option value="${p.n}" ${+vs.phase === p.n ? 'selected' : ''}>${L('Phase', 'Fase')} ${p.n} · ${esc(L(p.en, p.es))}</option>`).join('')}</select>
      <div class="pill-row">${['all', 'due', 'learning', 'remembered', 'new'].map(s => `<button type="button" class="pill ${vs.status === s ? 'active' : ''}" data-action="vocab-status" data-v="${s}">${s === 'all' ? L('All', 'Todas') : esc(L(...LABEL[s]))}</button>`).join('')}</div>
      <p class="legend"><span class="art art-m">el</span> ${L('masculine', 'masculino')} · <span class="art art-f">la</span> ${L('feminine', 'femenino')}</p>
    </section>
    ${groups.length ? groups.map(({ u, words }) => `<section class="card vocab-unit"><div class="card-head"><h2 class="card-title">${L(`Lesson ${u.n}`, `Lección ${u.n}`)} · ${esc(L(u.title, u.es))}</h2><button type="button" class="text-link" data-go="lesson/${u.id}">${L('Open lesson ›', 'Abrir lección ›')}</button></div>
      <div class="vocab-table">${words.map(v => { const s = status(S, v.id, today); return `<div class="vocab-line"><div class="word-left">${wordHTML(v)}${speakBtn(v.es)}</div><span class="translation">${esc(v.en)}</span><span class="vocab-ex" lang="es">${esc(v.ex || '')}</span><span class="state-badge ${s === 'remembered' ? 'done' : s === 'due' ? 'current' : ''}">${esc(L(...LABEL[s]))}</span></div>`; }).join('')}</div></section>`).join('')
      : LQ.ui.emptyState('search', L('No words found', 'No se encontraron palabras'), L('Try another search or filter.', 'Prueba otra búsqueda o filtro.'), '', 'empty-search')}
    <p class="card-sub center">${L(`${count} words shown`, `${count} palabras`)}</p>`;
  };

  LQ.inputs['vocab-q'] = el => { vs.q = el.value; LQ.app.render(); };
  LQ.changes['vocab-phase'] = el => { vs.phase = el.value; LQ.app.render(); };
  LQ.actions['vocab-status'] = b => { vs.status = b.dataset.v; LQ.app.render(); };
})(typeof window !== 'undefined' ? window : globalThis);
