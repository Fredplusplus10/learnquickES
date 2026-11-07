/* First run: welcome → about you → placement test → plan → voice → how it works. Restore a backup at step 0. */
(function (root) {
  'use strict';
  const LQ = root.LQ;
  const U = LQ.util, C = LQ.content, CAL = LQ.calendar;
  const { ico, esc } = LQ.ui;
  const L = (en, es) => LQ.L(en, es);
  const V = LQ.views = LQ.views || {};
  LQ.actions = LQ.actions || {}; LQ.changes = LQ.changes || {}; LQ.inputs = LQ.inputs || {};
  let os = { step: 0 };

  const dots = () => `<div class="onb-dots" aria-hidden="true">${[0, 1, 2, 3, 4, 5].map(i => `<span class="${i === os.step ? 'now' : i < os.step ? 'on' : ''}"></span>`).join('')}</div>`;
  const nav = (next = true, label) => `<div class="bottom-buttons">${os.step > 0 ? `<button type="button" class="secondary-btn left-button" data-action="onb-step" data-step="${os.step - 1}">${ico('chevron-left')}${L('Back', 'Atrás')}</button>` : ''}${next ? `<button type="button" class="primary-btn" data-action="onb-step" data-step="${os.step + 1}">${label || L('Continue', 'Continuar')} ${ico('chevron-right', 'white')}</button>` : ''}</div>`;
  const PLAN_TEXT = {
    30: [['Intensive', 'Intensivo'], ['Best if you already handle everyday conversations (solid A2). Lessons every day plus a lot of listening.', 'Ideal si ya te defiendes en conversaciones cotidianas (A2 sólido). Lección cada día y mucha escucha.']],
    60: [['Balanced', 'Equilibrado'], ['Recommended for most A2 learners: lessons with practice days in between.', 'Recomendado para la mayoría de alumnos A2: lecciones con días de práctica entre medias.']],
    90: [['Steady', 'Constante'], ['Best from A1 or after a long break: more practice days, a calmer pace.', 'Ideal desde A1 o tras mucho tiempo sin estudiar: más días de práctica y un ritmo tranquilo.']]
  };

  V.onboarding = (param, ctx) => {
    const S = LQ.state;
    if (ctx && ctx.fresh && !os.started) os = { step: 0, started: true };
    let body = '';
    switch (os.step) {
      case 0: body = `<img class="onb-art" src="assets/illustrations/spanish-city.svg" alt="" aria-hidden="true">
        <h1>¡Bienvenido a LearnQuick! <span class="flag" aria-hidden="true"><img src="assets/icons/spanish-flag.svg" alt=""></span></h1>
        <p class="lead">${L('A personal plan to reach functional B1 Spanish in 30–90 days: 25–45 minutes a day in the app, plus listening and real conversation.', 'Un plan personal para llegar a un español B1 funcional en 30–90 días: 25–45 minutos al día en la app, más escucha y conversación real.')}</p>
        <ul class="onb-list"><li>${ico('flame')}${L('A daily streak that respects weekends and holidays', 'Una racha diaria que respeta fines de semana y festivos')}</li><li>${ico('bolt')}${L('The 80/20 verbs, cognates and fui vs. iba — the hacks that matter', 'Los verbos 80/20, los cognados y fui vs. iba: los trucos que importan')}</li><li>${ico('mic')}${L('Speaking from day one with shadowing', 'Hablar desde el primer día con shadowing')}</li><li>${ico('shield')}${L('100% offline and private — your data never leaves this Mac', '100 % sin conexión y privado: tus datos no salen de este Mac')}</li></ul>
        <div class="bottom-buttons center"><button type="button" class="primary-btn big" data-action="onb-step" data-step="1">${L("Let's start", 'Empezar')} ${ico('chevron-right', 'white')}</button>
          <label class="secondary-btn file-btn">${ico('upload')}${L('Restore a backup', 'Restaurar una copia')}<input type="file" accept=".json,application/json" data-change="restore" hidden></label>
          <a class="secondary-btn" href="index.html?demo">${ico('eye')}${L('Explore a demo', 'Ver una demo')}</a></div>`; break;
      case 1: body = `<h1>${L('Nice to meet you', 'Mucho gusto')}</h1><p class="lead">${L('What should we call you?', '¿Cómo te llamas?')}</p>
        <input type="text" id="onbName" class="text-input big" maxlength="30" value="${esc(S.profile.name)}" data-input="onb-name" placeholder="${L('Your first name', 'Tu nombre')}" autocomplete="given-name">
        <p class="mini-title">${L('Interface language', 'Idioma de la interfaz')}</p>
        <div class="pill-row"><button type="button" class="pill ${S.profile.uiLang === 'en' ? 'active' : ''}" data-action="ui-lang" data-v="en">English</button><button type="button" class="pill ${S.profile.uiLang === 'es' ? 'active' : ''}" data-action="ui-lang" data-v="es">Español (${L('more immersion', 'más inmersión')})</button></div>
        <p class="card-sub">${L('Exercises and content are always in Spanish. You can change this any time in Settings.', 'Los ejercicios y el contenido siempre están en español. Puedes cambiarlo cuando quieras en Ajustes.')}</p>${nav()}`; break;
      case 2:
        if (os.testing) return { html: frame(`<h1>${L('Placement test', 'Prueba de nivel')}</h1><div id="placementSession"></div>`), mount(el) {
          const items = C.placement.map(q => LQ.ex.mc(q.q, q.o, { skill: 'placement' }));
          const s = LQ.ex.run(el.querySelector('#placementSession'), { items, requeue: false, record: false, onFinish(res) { os.testing = false; os.score = res.first; const r = C.placementLevel(res.first); Object.assign(LQ.state.profile, { placement: res.first, level: r.level }); LQ.state.plan.days = r.plan; LQ.app.save(); os.step = 3; LQ.app.render(); }, onQuit() { os.testing = false; LQ.app.render(); } });
          return () => s.stop();
        } };
        body = `<h1>${L('Where are you starting from?', '¿Desde dónde empiezas?')}</h1><p class="lead">${L('15 quick questions (about 3 minutes) to recommend the right plan. No pressure — just answer what you know.', '15 preguntas rápidas (unos 3 minutos) para recomendarte el plan adecuado. Sin presión: responde lo que sepas.')}</p>
          <div class="bottom-buttons center"><button type="button" class="primary-btn big" data-action="onb-test">${ico('play', 'white')}${L('Take the placement test', 'Hacer la prueba de nivel')}</button></div>
          <p class="mini-title">${L('Or pick your level', 'O elige tu nivel')}</p><div class="pill-row">${[['A1', 90, ['Beginner / false beginner', 'Principiante']], ['A2', 60, ['I can handle simple conversations', 'Me defiendo en conversaciones sencillas']], ['A2+ / B1−', 30, ['I talk about the past with some errors', 'Hablo del pasado con algunos errores']]].map(([lv, d, t]) => `<button type="button" class="pill" data-action="onb-level" data-level="${lv}" data-days="${d}"><b>${lv}</b> · ${esc(L(...t))}</button>`).join('')}</div>${nav(false)}`; break;
      case 3: {
        const rec = S.profile.placement !== null ? C.placementLevel(S.profile.placement).plan : { A1: 90, A2: 60 }[S.profile.level] || (S.profile.level ? 30 : 60);
        body = `<h1>${L('Choose your plan', 'Elige tu plan')}</h1>
          ${S.profile.placement !== null ? `<p class="lead">${L(`You scored <b>${esc(S.profile.placement)}/15</b> — about <b>${esc(S.profile.level)}</b>. We recommend the <b>${rec}-day</b> plan.`, `Sacaste <b>${esc(S.profile.placement)}/15</b>: más o menos <b>${esc(S.profile.level)}</b>. Te recomendamos el plan de <b>${rec} días</b>.`)}</p>` : `<p class="lead">${L(`Recommended for you: the <b>${rec}-day</b> plan.`, `Recomendado para ti: el plan de <b>${rec} días</b>.`)}</p>`}
          <div class="plan-grid">${[30, 60, 90].map(d => `<button type="button" class="plan-card ${S.plan.days === d ? 'selected' : ''}" data-action="onb-plan" data-v="${d}">${d === rec ? `<span class="state-badge current">${L('Recommended', 'Recomendado')}</span>` : ''}<b>${d} ${L('study days', 'días de estudio')}</b><span class="plan-name">${esc(L(...PLAN_TEXT[d][0]))}</span>
            <span class="plan-mins">~${LQ.planMinutes(d)} ${L('min/day in the app', 'min/día en la app')} + ${CAL.INPUT_TARGET[d]} ${L('min listening', 'min de escucha')}</span><p>${esc(L(...PLAN_TEXT[d][1]))}</p></button>`).join('')}</div>
          <div class="setting-option"><div><b>${L('Start date', 'Fecha de inicio')}</b><small>${L('Monday to Friday by default; weekends and US federal holidays are free days. Change it in Settings.', 'De lunes a viernes por defecto; fines de semana y festivos federales son libres. Cámbialo en Ajustes.')}</small></div><input type="date" value="${S.plan.start}" data-change="plan-start"></div>
          <p class="card-sub">${L('Honest note: B1 in 30–90 days is realistic from an A2 base with daily practice and immersion. From zero, expect a solid A2 in 90 days — and keep going.', 'Nota honesta: el B1 en 30–90 días es realista desde una base A2 con práctica diaria e inmersión. Desde cero, espera un A2 sólido en 90 días, y sigue adelante.')}</p>${nav()}`; break;
      }
      case 4: body = `<h1>${L('Pick your Spanish voice', 'Elige tu voz en español')}</h1><p class="lead">${L('LearnQuick reads every word, sentence and dialogue aloud with the voices built into your Mac.', 'LearnQuick lee en voz alta cada palabra, frase y diálogo con las voces de tu Mac.')}</p><div class="card">${LQ.voiceBlock(S)}</div>${nav()}`; break;
      case 5: body = `<h1>${L('How LearnQuick works', 'Cómo funciona LearnQuick')}</h1>
        <div class="how-grid">
          <div>${ico('calendar')}<b>${L('A plan for every day', 'Un plan para cada día')}</b><p>${L('Open the app and press "Continue today". Each day has 3–4 short tasks.', 'Abre la app y pulsa «Seguir con hoy». Cada día tiene 3 o 4 tareas cortas.')}</p></div>
          <div>${ico('flame')}<b>${L('Your streak', 'Tu racha')}</b><p>${L('Earn 10 XP on a day to keep it. Weekends, holidays and days off never break it, and streak freezes protect a missed study day.', 'Gana 10 XP en un día para mantenerla. Los fines de semana, festivos y días libres nunca la rompen, y los protectores cubren un día perdido.')}</p></div>
          <div>${ico('headphones')}<b>${L('Immersion counts', 'La inmersión cuenta')}</b><p>${L('Log podcasts, series and conversations. Volume of Spanish is what gets you to B1.', 'Registra pódcasts, series y conversaciones. El volumen de español es lo que te lleva al B1.')}</p></div>
          <div>${ico('download')}<b>${L('Back up weekly', 'Copia semanal')}</b><p>${L('Your progress lives on this Mac. One click in Settings saves a backup file.', 'Tu progreso vive en este Mac. Un clic en Ajustes guarda una copia.')}</p></div>
        </div>
        <div class="coach-tip"><b>${L('Your first mission (trick 2):', 'Tu primera misión (truco 2):')}</b> ${L('switch your iPhone to Spanish — Settings → General → Language & Region → Español (México). It turns every notification into practice.', 'pon tu iPhone en español: Ajustes → General → Idioma y región → Español (México). Cada notificación se vuelve práctica.')}</div>
        <div class="bottom-buttons"><button type="button" class="secondary-btn left-button" data-action="onb-step" data-step="4">${ico('chevron-left')}${L('Back', 'Atrás')}</button><button type="button" class="primary-btn big" data-action="onb-finish">${ico('play', 'white')}${L('Start Day 1', 'Empezar el día 1')}</button></div>`; break;
    }
    return frame(body);
  };
  const frame = body => `<div class="onboarding">${dots()}<section class="card onb-card">${body}</section></div>`;

  Object.assign(LQ.actions, {
    'onb-step'(b) { os.step = +b.dataset.step; LQ.app.render(); },
    'onb-test'() { os.testing = true; LQ.app.render(); },
    'onb-level'(b) { Object.assign(LQ.state.profile, { level: b.dataset.level, placement: null }); LQ.state.plan.days = +b.dataset.days; LQ.app.save(); os.step = 3; LQ.app.render(); },
    'onb-plan'(b) { LQ.state.plan.days = +b.dataset.v; LQ.app.save(); LQ.app.render(); },
    'onb-finish'() {
      const S = LQ.state; S.onboarded = true;
      if (S.plan.start < U.today()) S.plan.start = U.today();
      LQ.app.save(); os = { step: 0 }; LQ.app.go('today');
    }
  });
  LQ.inputs['onb-name'] = el => { LQ.state.profile.name = el.value.trim().slice(0, 30); LQ.app.save(); };
})(typeof window !== 'undefined' ? window : globalThis);
