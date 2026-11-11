/* Audio services: text-to-speech with the Mac's Spanish voices, optional speech recognition and recording.
   Every feature is detected at use time and degrades to a clear message — the lesson never blocks on audio. */
(function (root) {
  'use strict';
  const LQ = root.LQ;
  const synth = 'speechSynthesis' in root ? root.speechSynthesis : null;
  const SR = root.SpeechRecognition || root.webkitSpeechRecognition || null;
  const localRecognition = !!(SR && SR.prototype && 'processLocally' in SR.prototype);
  // "Check words" is announced, not offered yet. Recognition on the device needs a Spanish language pack that
  // browsers do not bring (Chrome 157 has it as a download, Edge 157 does not have it), and the app cannot
  // install it yet: every try ended in "language-not-supported". True once the app can.
  const RECOGNITION_READY = false;
  let voices = [];
  const norm = s => String(s || '').replace('_', '-').toLowerCase();
  function loadVoices() { if (synth) voices = synth.getVoices().filter(v => /^es(-|$)/.test(norm(v.lang))); }
  if (synth) { loadVoices(); synth.addEventListener && synth.addEventListener('voiceschanged', loadVoices); }

  const LATAM = /^es-(mx|us|419|co|ar|cl|pe|ve|uy|ec|gt|cr|pr|do|bo|py|sv|hn|ni|pa|cu)$/;
  // Prefer the chosen voice, then the accent, then higher-quality (Enhanced/Premium) local voices.
  function pickVoice() {
    const p = LQ.state.profile;
    if (p.voiceURI) { const v = voices.find(x => x.voiceURI === p.voiceURI); if (v) return v; }
    const want = norm(p.accent);
    let pool = voices.filter(v => norm(v.lang) === want);
    if (!pool.length && want !== 'es-es') pool = voices.filter(v => LATAM.test(norm(v.lang)));
    if (!pool.length) pool = voices;
    return pool.find(v => /premium|enhanced|mejorad|siri/i.test(v.name)) || pool.find(v => v.localService) || pool[0] || null;
  }

  let token = 0;
  // Long texts are split by sentence: some engines silently stop utterances after ~15 seconds.
  function speak(text, opts = {}) {
    if (!synth) { LQ.ui.toast(LQ.L('Speech is not available in this browser. Read the text instead.', 'La voz no está disponible en este navegador. Lee el texto.')); opts.onend && opts.onend(); return false; }
    synth.cancel();
    // No regex lookbehind here: it is a syntax error on Safari < 16.4 and would break the whole app.
    const my = ++token, parts = (String(text).match(/[^.!?…]+[.!?…]*/g) || [String(text)]).map(s => s.trim()).filter(Boolean), v = pickVoice();
    parts.forEach((part, i) => {
      const u = new SpeechSynthesisUtterance(part);
      if (v) { u.voice = v; u.lang = v.lang; } else u.lang = LQ.state.profile.accent;
      u.rate = opts.rate || LQ.state.profile.rate || 0.9;
      if (i === parts.length - 1) u.onend = () => { if (my === token && opts.onend) opts.onend(); };
      u.onerror = e => { if (!['interrupted', 'canceled'].includes(e.error)) LQ.ui.toast(LQ.L('Spanish speech could not be played. Check Settings → Voice.', 'No se pudo reproducir la voz. Revisa Ajustes → Voz.')); if (my === token && opts.onend) opts.onend(); };
      synth.speak(u);
    });
    if (!voices.length && !speak.warned) { speak.warned = true; LQ.ui.toast(LQ.L('No Spanish voice found — using the default voice. See Settings → Voice.', 'No hay voz en español: se usa la voz por defecto. Mira Ajustes → Voz.')); }
    return true;
  }
  const stop = () => { token++; synth && synth.cancel(); };

  // Speech recognition: shows what the computer understood. It is NOT a pronunciation score.
  function recognize(onResult, onError) {
    // Older browser engines may send transcripts to a remote provider. This app is offline-first.
    if (!localRecognition) { onError('local-unavailable'); return null; }
    const r = new SR();
    r.processLocally = true;
    r.lang = LQ.state.profile.accent; r.interimResults = false; r.maxAlternatives = 3;
    r.onresult = e => onResult(Array.from(e.results[0]).map(a => a.transcript));
    r.onerror = e => onError(e.error || 'error');
    try { r.start(); } catch { onError('start'); return null; }
    return r;
  }
  // Word overlap between the target phrase and what was recognized (accent-insensitive).
  function compare(target, heard) {
    const U = LQ.util, key = w => U.stripAccents(w);
    const got = new Set(U.words(heard).map(key));
    const words = U.words(target).map(w => ({ w, ok: got.has(key(w)) }));
    return { words, hits: words.filter(x => x.ok).length };
  }

  const canRecord = () => !!(navigator.mediaDevices && navigator.mediaDevices.getUserMedia && root.MediaRecorder);
  // Records until stop() or maxMs; resolves with an object URL to play back. Audio never leaves the device.
  async function record(maxMs = 12000) {
    const stream = await navigator.mediaDevices.getUserMedia({ audio: true });
    const release = () => stream.getTracks().forEach(t => t.stop());
    let rec;
    try { rec = new MediaRecorder(stream); }
    catch (e) { release(); throw e; }
    const chunks = [];
    rec.ondataavailable = e => e.data.size && chunks.push(e.data);
    const done = new Promise((resolve, reject) => {
      rec.onstop = () => { release(); resolve(URL.createObjectURL(new Blob(chunks, { type: rec.mimeType || 'audio/webm' }))); };
      rec.onerror = e => { release(); reject(e.error || new Error('Microphone recording failed')); };
    });
    try { rec.start(); }
    catch (e) { release(); throw e; }
    const timer = setTimeout(() => { if (rec.state === 'recording') rec.stop(); }, Math.max(1000, Math.min(60000, maxMs)));
    done.finally(() => clearTimeout(timer)).catch(() => {});
    return { stop() { clearTimeout(timer); if (rec.state === 'recording') rec.stop(); }, done };
  }


  LQ.speech = { speak, stop, pickVoice, voices: () => voices, hasTTS: !!synth, hasRecognition: RECOGNITION_READY && localRecognition, recognitionSoon: !RECOGNITION_READY && !!SR, recognize, compare, canRecord, record };
})(typeof window !== 'undefined' ? window : globalThis);
