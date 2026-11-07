/* Verb data + conjugator. Regular forms come from rules; irregular verbs list only the tenses that deviate.
   Persons: 0 yo · 1 tú · 2 él/ella/usted · 3 nosotros · 4 vosotros · 5 ellos/ellas/ustedes. */
(function (root) {
  'use strict';
  const LQ = root.LQ = root.LQ || {};

  const ENDINGS = {
    pres: { ar: ['o', 'as', 'a', 'amos', 'áis', 'an'], er: ['o', 'es', 'e', 'emos', 'éis', 'en'], ir: ['o', 'es', 'e', 'imos', 'ís', 'en'] },
    pret: { ar: ['é', 'aste', 'ó', 'amos', 'asteis', 'aron'], er: ['í', 'iste', 'ió', 'imos', 'isteis', 'ieron'], ir: ['í', 'iste', 'ió', 'imos', 'isteis', 'ieron'] },
    imp: { ar: ['aba', 'abas', 'aba', 'ábamos', 'abais', 'aban'], er: ['ía', 'ías', 'ía', 'íamos', 'íais', 'ían'], ir: ['ía', 'ías', 'ía', 'íamos', 'íais', 'ían'] },
    subj: { ar: ['e', 'es', 'e', 'emos', 'éis', 'en'], er: ['a', 'as', 'a', 'amos', 'áis', 'an'], ir: ['a', 'as', 'a', 'amos', 'áis', 'an'] }
  };
  const FUT = ['é', 'ás', 'á', 'emos', 'éis', 'án'], COND = ['ía', 'ías', 'ía', 'íamos', 'íais', 'ían'];
  const HABER = ['he', 'has', 'ha', 'hemos', 'habéis', 'han'];
  const IR_PRES = ['voy', 'vas', 'va', 'vamos', 'vais', 'van'];
  const w = s => s.split(' ');

  // core: the 80/20 verbs from the method. Only deviations are listed; everything else is generated.
  const VERBS = {
    ser: { en: 'to be (identity)', core: 1, pres: w('soy eres es somos sois son'), pret: w('fui fuiste fue fuimos fuisteis fueron'), imp: w('era eras era éramos erais eran'), subj: w('sea seas sea seamos seáis sean'), part: 'sido' },
    estar: { en: 'to be (state, place)', core: 1, pres: w('estoy estás está estamos estáis están'), pret: w('estuve estuviste estuvo estuvimos estuvisteis estuvieron'), subj: w('esté estés esté estemos estéis estén') },
    tener: { en: 'to have', core: 1, pres: w('tengo tienes tiene tenemos tenéis tienen'), pret: w('tuve tuviste tuvo tuvimos tuvisteis tuvieron'), stem: 'tendr' },
    hacer: { en: 'to do, make', core: 1, pres: w('hago haces hace hacemos hacéis hacen'), pret: w('hice hiciste hizo hicimos hicisteis hicieron'), stem: 'har', part: 'hecho' },
    poder: { en: 'to be able to, can', core: 1, pres: w('puedo puedes puede podemos podéis pueden'), pret: w('pude pudiste pudo pudimos pudisteis pudieron'), stem: 'podr', subj: w('pueda puedas pueda podamos podáis puedan') },
    querer: { en: 'to want; to love', core: 1, pres: w('quiero quieres quiere queremos queréis quieren'), pret: w('quise quisiste quiso quisimos quisisteis quisieron'), stem: 'querr', subj: w('quiera quieras quiera queramos queráis quieran') },
    ir: { en: 'to go', core: 1, pres: IR_PRES, pret: w('fui fuiste fue fuimos fuisteis fueron'), imp: w('iba ibas iba íbamos ibais iban'), subj: w('vaya vayas vaya vayamos vayáis vayan') },
    decir: { en: 'to say, tell', core: 1, pres: w('digo dices dice decimos decís dicen'), pret: w('dije dijiste dijo dijimos dijisteis dijeron'), stem: 'dir', part: 'dicho' },
    hablar: { en: 'to speak' },
    comer: { en: 'to eat' },
    vivir: { en: 'to live' },
    trabajar: { en: 'to work' },
    ver: { en: 'to see, watch', pres: w('veo ves ve vemos veis ven'), pret: w('vi viste vio vimos visteis vieron'), imp: w('veía veías veía veíamos veíais veían'), part: 'visto' },
    dar: { en: 'to give', pres: w('doy das da damos dais dan'), pret: w('di diste dio dimos disteis dieron'), subj: w('dé des dé demos deis den') },
    saber: { en: 'to know (facts)', pres: w('sé sabes sabe sabemos sabéis saben'), pret: w('supe supiste supo supimos supisteis supieron'), stem: 'sabr', subj: w('sepa sepas sepa sepamos sepáis sepan') },
    venir: { en: 'to come', pres: w('vengo vienes viene venimos venís vienen'), pret: w('vine viniste vino vinimos vinisteis vinieron'), stem: 'vendr' },
    salir: { en: 'to go out, leave', pres: w('salgo sales sale salimos salís salen'), stem: 'saldr' },
    poner: { en: 'to put', pres: w('pongo pones pone ponemos ponéis ponen'), pret: w('puse pusiste puso pusimos pusisteis pusieron'), stem: 'pondr', part: 'puesto' },
    llegar: { en: 'to arrive', pret: w('llegué llegaste llegó llegamos llegasteis llegaron'), subj: w('llegue llegues llegue lleguemos lleguéis lleguen') },
    pensar: { en: 'to think', pres: w('pienso piensas piensa pensamos pensáis piensan'), subj: w('piense pienses piense pensemos penséis piensen') },
    volver: { en: 'to return', pres: w('vuelvo vuelves vuelve volvemos volvéis vuelven'), subj: w('vuelva vuelvas vuelva volvamos volváis vuelvan'), part: 'vuelto' },
    conocer: { en: 'to know (people, places)', pres: w('conozco conoces conoce conocemos conocéis conocen') },
    sentir: { en: 'to feel', pres: w('siento sientes siente sentimos sentís sienten'), pret: w('sentí sentiste sintió sentimos sentisteis sintieron'), subj: w('sienta sientas sienta sintamos sintáis sientan') },
    pedir: { en: 'to ask for, order', pres: w('pido pides pide pedimos pedís piden'), pret: w('pedí pediste pidió pedimos pedisteis pidieron') },
    dormir: { en: 'to sleep', pres: w('duermo duermes duerme dormimos dormís duermen'), pret: w('dormí dormiste durmió dormimos dormisteis durmieron'), subj: w('duerma duermas duerma durmamos durmáis duerman') }
  };

  const TENSES = {
    pres: { en: 'Present', es: 'Presente', cue: 'hoy · normalmente', unit: 0 },
    irA: { en: 'Near future (ir a + inf.)', es: 'Futuro perifrástico', cue: 'mañana · plan', unit: 3 },
    pret: { en: 'Preterite (fui)', es: 'Pretérito', cue: 'ayer · una vez', unit: 10 },
    imp: { en: 'Imperfect (iba)', es: 'Imperfecto', cue: 'antes · de niño', unit: 12 },
    perf: { en: 'Present perfect', es: 'Pretérito perfecto', cue: 'alguna vez · hoy', unit: 15 },
    fut: { en: 'Future', es: 'Futuro', cue: 'el año que viene', unit: 16 },
    cond: { en: 'Conditional', es: 'Condicional', cue: 'cortesía · consejo', unit: 17 },
    subj: { en: 'Present subjunctive', es: 'Subjuntivo presente', cue: 'Quiero que…', unit: 20 }
  };
  const PERSONS = [['yo'], ['tú'], ['él', 'ella', 'usted'], ['nosotros', 'nosotras'], ['vosotros', 'vosotras'], ['ellos', 'ellas', 'ustedes']];

  const type = inf => inf.slice(-2);
  const stemOf = inf => inf.slice(0, -2);
  function participle(inf) { const v = VERBS[inf] || {}; return v.part || stemOf(inf) + (type(inf) === 'ar' ? 'ado' : 'ido'); }

  // Full 6-person paradigm for a tense.
  function conjugate(inf, tense) {
    const v = VERBS[inf] || {}, t = type(inf), stem = stemOf(inf);
    if (v[tense]) return v[tense].slice();
    switch (tense) {
      case 'pres': case 'pret': case 'imp': return ENDINGS[tense][t].map(e => stem + e);
      case 'fut': return FUT.map(e => (v.stem || inf) + e);
      case 'cond': return COND.map(e => (v.stem || inf) + e);
      case 'irA': return IR_PRES.map(f => `${f} a ${inf}`);
      case 'perf': return HABER.map(h => `${h} ${participle(inf)}`);
      case 'subj': { const yo = conjugate(inf, 'pres')[0]; return ENDINGS.subj[t].map(e => yo.slice(0, -1) + e); } // yo form minus -o
    }
    throw new Error('Unknown tense ' + tense);
  }
  const form = (inf, tense, person) => conjugate(inf, tense)[person];

  LQ.verbs = { VERBS, TENSES, PERSONS, conjugate, form, participle, CORE: Object.keys(VERBS).filter(k => VERBS[k].core) };
  if (typeof module !== 'undefined') module.exports = LQ;
})(typeof window !== 'undefined' ? window : globalThis);
