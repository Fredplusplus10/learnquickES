/* Shared UI building blocks. Templates are strings (no framework), events use delegation on data attributes. */
(function (root) {
  'use strict';
  const LQ = root.LQ;
  const U = LQ.util;
  const esc = U.esc;

  // Bilingual UI copy: English by default, Spanish in immersion mode or during a language-island session.
  LQ.lang = () => (LQ.island && LQ.island.active) || LQ.state.profile.uiLang === 'es' ? 'es' : 'en';
  LQ.L = (en, es) => LQ.lang() === 'es' ? es : en;
  const L = LQ.L;

  const ico = (name, cls = '') => `<img class="ico ${cls}" src="assets/icons/${name}.svg" alt="" aria-hidden="true">`;
  const es = (text, tag = 'span', cls = '') => `<${tag} lang="es" class="${cls}">${esc(text)}</${tag}>`;
  const speakBtn = (text, label) => `<button type="button" class="speak-button" data-speak="${esc(text)}" aria-label="${esc(label || L('Listen: ', 'Escuchar: ') + text)}">${ico('speaker')}</button>`;
  const genderTag = g => g ? `<span class="g-tag g-${g}" title="${g === 'm' ? L('masculine', 'masculino') : L('feminine', 'femenino')}">${g === 'm' ? 'el' : 'la'}</span>` : '';
  // Spanish word with its article coloured by gender (the article text itself carries the information).
  function wordHTML(v) {
    const m = /^(el|la|los|las)\s+(.*)$/i.exec(v.es);
    return m ? `<b lang="es"><span class="art art-${v.g}">${esc(m[1])}</span> ${esc(m[2])}</b>` : `<b lang="es">${esc(v.es)}</b>`;
  }
  const wordRow = v => `<div class="word-row"><div class="word-left">${wordHTML(v)}${speakBtn(v.es)}</div><span class="translation">${esc(v.en)}</span></div>`;

  function ring(pct, label) { return `<div class="ring" style="--pct:${Math.round(pct)}%"><div class="ring-inner"><b>${Math.round(pct)}%</b><small>${esc(label)}</small></div></div>`; }
  function meter(pct, label) { return `<div class="meter" role="progressbar" aria-valuenow="${Math.round(pct)}" aria-valuemin="0" aria-valuemax="100" aria-label="${esc(label)}"><span style="width:${U.clamp(pct, 0, 100)}%"></span></div>`; }
  const DOW = () => LQ.lang() === 'es' ? ['Lun', 'Mar', 'Mié', 'Jue', 'Vie', 'Sáb', 'Dom'] : ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'];
  const STATE_LABEL = { done: ['studied', 'estudiado'], frozen: ['protected by a freeze', 'protegido'], missed: ['missed', 'perdido'], off: ['day off', 'día libre'], today: ['today', 'hoy'], future: ['upcoming', 'próximo'] };
  function streakWeek(week) {
    return `<div class="streak-row">${week.map((d, i) => {
      const mark = d.state === 'done' ? '✓' : d.state === 'frozen' ? ico('shield') : d.state === 'missed' ? '·' : '';
      return `<div><span>${DOW()[i]}</span><div class="day-dot ${d.state}${d.scheduled ? '' : ' unscheduled'}" title="${esc(L(...STATE_LABEL[d.state]))}">${mark}</div></div>`;
    }).join('')}</div>`;
  }
  // `art` names an illustration in assets/illustrations (empty-*.svg) to show instead of the icon.
  const emptyState = (icon, title, text, action = '', art = '') => `<div class="empty-state">${art ? `<img class="empty-art" src="assets/illustrations/${art}.svg" alt="">` : ico(icon)}<h2>${esc(title)}</h2><p>${esc(text)}</p>${action}</div>`;

  // ---- Toasts, modal, celebrations -----------------------------------------------------------------------
  // Toasts queue up so a freeze notice and an achievement shown together are both readable.
  const toasts = []; let toastTimer = null, toastNow = null;
  function toast(msg) {
    // Skip repeats of the message on screen or already waiting (e.g. no Spanish voice, many clicks).
    if (msg === toastNow || toasts.includes(msg)) return;
    toasts.push(msg); if (!toastTimer) nextToast();
  }
  function nextToast() {
    const t = document.getElementById('toast'), msg = toasts.shift();
    if (!t || msg === undefined) { if (t) t.hidden = true; toastTimer = null; toastNow = null; return; }
    toastNow = msg;
    t.textContent = msg; t.hidden = false;
    toastTimer = setTimeout(nextToast, toasts.length ? 2600 : 4200);
  }
  function modal(html, { wide = false, onClose } = {}) {
    const d = document.getElementById('modal');
    d.classList.toggle('wide', wide);
    document.getElementById('modalContent').innerHTML = html;
    d.onclose = () => { onClose && onClose(); document.getElementById('modalContent').innerHTML = ''; };
    if (!d.open) d.showModal();
    return d;
  }
  const closeModal = () => { const d = document.getElementById('modal'); if (d.open) d.close(); };

  // Celebration overlay for streak extensions and completed days. Queued so it never interrupts an exercise.
  const queue = [];
  function celebrate(c) { queue.push(c); }
  function flushCelebrations() {
    if (!queue.length || document.getElementById('modal').open) return;
    const items = queue.splice(0), streak = items.find(c => c.type === 'streak'), day = items.find(c => c.type === 'dayComplete'), phase = items.find(c => c.type === 'phaseComplete');
    const st = LQ.progress.streakInfo(LQ.state, U.today());
    const title = streak ? L(`${streak.count}-day streak!`, `¡Racha de ${streak.count} días!`) : day ? L(`Day ${day.n} complete!`, `¡Día ${day.n} completado!`) : '';
    if (!title) return;
    const lines = [];
    if (day) lines.push(L(`You finished day ${day.n} of your plan.`, `Terminaste el día ${day.n} de tu plan.`));
    if (phase) lines.push(L(`Phase ${phase.phase} complete — ${LQ.content.phases[phase.phase - 1].en}.`, `Fase ${phase.phase} completada: ${LQ.content.phases[phase.phase - 1].es}.`));
    if (streak && !day) lines.push(L('You earned today\'s streak. Finish today\'s plan to move forward.', 'Ya tienes la racha de hoy. Termina el plan de hoy para avanzar.'));
    modal(`<div class="celebrate">
      <div class="celebrate-flame">${ico('flame')}<b>${st.count}</b></div>
      <h2>${esc(title)}</h2>${lines.map(l => `<p>${esc(l)}</p>`).join('')}
      ${streakWeek(st.week)}
      <p class="freeze-line">${ico('shield')} ${L(`${st.freezes} of 2 streak freezes`, `${st.freezes} de 2 protectores de racha`)}</p>
    </div>`);
  }

  // ---- Inputs ----------------------------------------------------------------------------------------------
  const ACCENTS = ['á', 'é', 'í', 'ó', 'ú', 'ñ', 'ü', '¿', '¡'];
  const accentBar = target => `<div class="accent-bar" role="group" aria-label="${L('Insert accented letter', 'Insertar letra con acento')}">${ACCENTS.map(c => `<button type="button" class="accent-key" data-accent="${c}" data-target="${target}">${c}</button>`).join('')}<small>${L('Mac tip: hold a vowel key to choose its accent.', 'En Mac: mantén pulsada una vocal para elegir el acento.')}</small></div>`;
  function insertAccent(btn) {
    const input = document.getElementById(btn.dataset.target); if (!input) return;
    const s = input.selectionStart ?? input.value.length, e = input.selectionEnd ?? input.value.length;
    input.value = input.value.slice(0, s) + btn.dataset.accent + input.value.slice(e);
    input.focus(); input.setSelectionRange(s + 1, s + 1);
    input.dispatchEvent(new Event('input', { bubbles: true }));
  }

  // ---- Shadowing block: listen → repeat aloud → (optional) record & compare / recognition -----------------
  let shadowSeq = 0;
  function shadow(phrase, en) {
    const id = 'sh' + (++shadowSeq);
    return `<div class="shadow" id="${id}" data-phrase="${esc(phrase)}">
      <p class="shadow-phrase" lang="es">${esc(phrase)}</p>${en ? `<p class="shadow-en">${esc(en)}</p>` : ''}
      <div class="shadow-actions">
        <button type="button" class="secondary-btn" data-speak="${esc(phrase)}">${ico('speaker')}${L('Listen', 'Escuchar')}</button>
        <button type="button" class="secondary-btn" data-speak="${esc(phrase)}" data-rate="0.65">${ico('turtle')}${L('Slow', 'Lento')}</button>
        <button type="button" class="secondary-btn" data-action="shadow-repeat" data-target="${id}">${ico('refresh')}${L('Listen & repeat', 'Escucha y repite')}</button>
        ${LQ.speech.canRecord() ? `<button type="button" class="secondary-btn" data-action="shadow-record" data-target="${id}">${ico('mic')}${L('Record me', 'Grabarme')}</button>` : ''}
        ${LQ.speech.hasRecognition ? `<button type="button" class="secondary-btn" data-action="shadow-recognize" data-target="${id}">${ico('waveform')}${L('Check words', 'Comprobar')}</button>` : ''}
      </div>
      <div class="shadow-out" aria-live="polite"></div>
    </div>`;
  }
  // Plays the phrase, then gives a visible window (≈1.4× its length) to repeat it out loud.
  function shadowRepeat(box) {
    const out = box.querySelector('.shadow-out'), phrase = box.dataset.phrase, t0 = Date.now();
    out.innerHTML = `<span class="pill-note">${L('Listen…', 'Escucha…')}</span>`;
    LQ.speech.speak(phrase, {
      onend() {
        const ms = U.clamp((Date.now() - t0) * 1.4, 1800, 9000);
        out.innerHTML = `<div class="repeat-bar"><span style="animation-duration:${ms}ms"></span></div><span class="pill-note">${L('Your turn — say it out loud!', '¡Tu turno! Dilo en voz alta.')}</span>`;
        setTimeout(() => { if (out.isConnected) out.innerHTML = `<span class="pill-note good">${L('Great. Repeat it 2–3 times, a little faster each time.', 'Muy bien. Repítelo 2 o 3 veces, cada vez más rápido.')}</span>`; }, ms);
      }
    });
  }
  async function shadowRecord(box, btn) {
    const out = box.querySelector('.shadow-out');
    if (box._rec) { box._rec.stop(); return; }
    try {
      box._rec = await LQ.speech.record(12000);
      btn.classList.add('recording'); btn.lastChild.textContent = L('Stop', 'Parar');
      out.innerHTML = `<span class="pill-note">${L('Recording… say the phrase.', 'Grabando… di la frase.')}</span>`;
      const url = await box._rec.done;
      box._rec = null; btn.classList.remove('recording'); btn.lastChild.textContent = L('Record me', 'Grabarme');
      out.innerHTML = `<div class="compare"><span>${L('You:', 'Tú:')}</span><audio controls src="${url}"></audio><button type="button" class="secondary-btn" data-speak="${esc(box.dataset.phrase)}">${ico('speaker')}${L('Model', 'Modelo')}</button></div><span class="pill-note">${L('Compare your vowels and rhythm with the model.', 'Compara tus vocales y tu ritmo con el modelo.')}</span>`;
    } catch (e) {
      box._rec = null;
      out.innerHTML = `<span class="pill-note bad">${L('Recording is not available here (the browser blocked the microphone). Use "Listen & repeat" — it works everywhere. Safari usually allows recording; Chrome does not when the app is opened as a file.', 'La grabación no está disponible aquí (el navegador bloqueó el micrófono). Usa «Escucha y repite», que funciona siempre. Safari suele permitir grabar; Chrome no, si la app se abre como archivo.')}</span>`;
    }
  }
  function shadowRecognize(box) {
    const out = box.querySelector('.shadow-out'), phrase = box.dataset.phrase;
    out.innerHTML = `<span class="pill-note">${L('Listening… say the phrase now.', 'Te escucho… di la frase ahora.')}</span>`;
    LQ.speech.recognize(alts => {
      const best = alts.map(a => ({ a, r: LQ.speech.compare(phrase, a) })).sort((x, y) => y.r.hits - x.r.hits)[0];
      out.innerHTML = `<p class="recog">${best.r.words.map(w => `<span class="${w.ok ? 'hit' : 'miss'}">${esc(w.w)}</span>`).join(' ')}</p>
        <span class="pill-note">${L(`The computer understood ${best.r.hits} of ${best.r.words.length} words: “${best.a}”. This is a rough check, not a pronunciation score.`, `El ordenador entendió ${best.r.hits} de ${best.r.words.length} palabras: «${best.a}». Es una comprobación aproximada, no una nota de pronunciación.`)}</span>`;
    }, err => {
      const msg = err === 'not-allowed' || err === 'service-not-allowed' ? L('Microphone or speech recognition is blocked in this browser.', 'El micrófono o el reconocimiento de voz están bloqueados en este navegador.') : err === 'no-speech' ? L('No speech detected. Try again a bit louder.', 'No se detectó voz. Inténtalo un poco más fuerte.') : L('Speech recognition is not available right now. Use "Listen & repeat" instead.', 'El reconocimiento de voz no está disponible ahora. Usa «Escucha y repite».');
      out.innerHTML = `<span class="pill-note bad">${esc(msg)}</span>`;
    });
  }

  LQ.ui = { ico, es, esc, speakBtn, genderTag, wordHTML, wordRow, ring, meter, streakWeek, emptyState, toast, modal, closeModal, celebrate, flushCelebrations, accentBar, insertAccent, shadow, shadowRepeat, shadowRecord, shadowRecognize };
})(typeof window !== 'undefined' ? window : globalThis);
