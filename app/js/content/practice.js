/* Trainer content: past tenses, cognates, false friends, gender, pronunciation, circumlocution, immersion,
   placement test and the B1 mock. Same 'mc' convention as units.js: the first option is the correct one. */
(function (root) {
  'use strict';
  const LQ = root.LQ = root.LQ || {};
  const C = LQ.content = LQ.content || {};

  // ---- Trick 5 · Fui vs. Iba ---------------------------------------------------------------------------
  C.pastReasons = {
    habit: 'Repeated habit in the past → imperfecto.', desc: 'Description of what something was like → imperfecto.',
    time: 'Clock time in the past → imperfecto.', age: 'Age in the past → imperfecto.', state: 'Ongoing state or feeling in the past → imperfecto.',
    background: 'Ongoing action in the background → imperfecto.', interrupt: 'The action that interrupts → pretérito.',
    specific: 'Completed action at a specific moment → pretérito.', sequence: 'A sequence of completed actions → pretérito.',
    event: 'Something suddenly happened → pretérito.', bounded: 'An action with a clear beginning and end → pretérito.', whole: 'A finished event seen as a whole → pretérito.'
  };
  C.past = [
    ['De niño, ___ al parque todos los domingos.', 'iba', 'fui', 'habit'], ['Ayer ___ al dentista a las tres.', 'fui', 'iba', 'specific'],
    ['___ las once de la noche.', 'Eran', 'Fueron', 'time'], ['Mientras ___ la cena, sonó el teléfono.', 'preparaba', 'preparé', 'background'],
    ['Mientras preparaba la cena, ___ el teléfono.', 'sonó', 'sonaba', 'interrupt'], ['La casa de mis abuelos ___ muy grande.', 'era', 'fue', 'desc'],
    ['El sábado pasado ___ una película buenísima.', 'vimos', 'veíamos', 'specific'], ['Cuando ___ veinte años, vivía en Boston.', 'tenía', 'tuve', 'age'],
    ['Llegué a casa, ___ y me acosté.', 'cené', 'cenaba', 'sequence'], ['Normalmente ___ a la oficina en bicicleta.', 'iba', 'fui', 'habit'],
    ['Esa mañana ___ mucho frío.', 'hacía', 'hizo', 'desc'], ['De repente, ___ a llover.', 'empezó', 'empezaba', 'event'],
    ['Estaba nervioso porque ___ un examen.', 'tenía', 'tuve', 'state'], ['En 2019 ___ a México por primera vez.', 'viajé', 'viajaba', 'specific'],
    ['Mi primer carro ___ rojo y muy viejo.', 'era', 'fue', 'desc'], ['Anoche no ___ bien.', 'dormí', 'dormía', 'specific'],
    ['Siempre ___ tarde a clase cuando era estudiante.', 'llegaba', 'llegué', 'habit'], ['___ por la calle cuando vi a Ana.', 'Caminaba', 'Caminé', 'background'],
    ['Cuando sonó la alarma, yo todavía ___.', 'dormía', 'dormí', 'background'], ['___ en Chicago durante cinco años, de 2015 a 2020.', 'Viví', 'Vivía', 'bounded'],
    ['El concierto de anoche ___ fantástico.', 'fue', 'era', 'whole'], ['Los niños ___ en el jardín mientras los padres hablaban.', 'jugaban', 'jugaron', 'background'],
    ['Ayer ___ tres horas en el aeropuerto.', 'esperé', 'esperaba', 'bounded'], ['Antes no me ___ el café, pero ahora me encanta.', 'gustaba', 'gustó', 'state'],
    ['Cuando era pequeño, mi familia ___ un perro.', 'tenía', 'tuvo', 'state'], ['Ella ___ la puerta, entró y saludó.', 'abrió', 'abría', 'sequence'],
    ['Eran las ocho y la tienda todavía ___ cerrada.', 'estaba', 'estuvo', 'desc'], ['Una vez ___ un oso en el bosque.', 'vi', 'veía', 'specific'],
    ['¿Qué ___ cuando te llamé?', 'hacías', 'hiciste', 'background'], ['Mi abuelo siempre nos ___ historias.', 'contaba', 'contó', 'habit'],
    ['El año pasado ___ un ascenso en mayo.', 'conseguí', 'conseguía', 'specific'], ['Hacía sol y los pájaros ___.', 'cantaban', 'cantaron', 'desc']
  ];

  // ---- Trick 1 · Cognates ------------------------------------------------------------------------------
  C.cognateRules = [
    ['-tion', '-ción', [['nation', 'nación'], ['information', 'información'], ['situation', 'situación'], ['education', 'educación']]],
    ['-sion', '-sión', [['decision', 'decisión'], ['television', 'televisión'], ['mission', 'misión']]],
    ['-ty', '-dad / -tad', [['city', 'ciudad'], ['university', 'universidad'], ['reality', 'realidad'], ['liberty', 'libertad']]],
    ['-ous', '-oso', [['delicious', 'delicioso'], ['famous', 'famoso'], ['nervous', 'nervioso'], ['curious', 'curioso']]],
    ['-ly', '-mente', [['exactly', 'exactamente'], ['finally', 'finalmente'], ['normally', 'normalmente'], ['rapidly', 'rápidamente']]],
    ['-ist', '-ista', [['artist', 'artista'], ['dentist', 'dentista'], ['tourist', 'turista']]],
    ['-ic', '-ico', [['public', 'público'], ['electric', 'eléctrico'], ['fantastic', 'fantástico'], ['typical', 'típico']]],
    ['-ive', '-ivo', [['active', 'activo'], ['creative', 'creativo'], ['positive', 'positivo'], ['negative', 'negativo']]],
    ['-ence / -ance', '-encia / -ancia', [['experience', 'experiencia'], ['difference', 'diferencia'], ['importance', 'importancia'], ['distance', 'distancia']]],
    ['-ary', '-ario', [['necessary', 'necesario'], ['dictionary', 'diccionario'], ['salary', 'salario']]],
    ['-ble', '-ble', [['possible', 'posible'], ['terrible', 'terrible'], ['flexible', 'flexible']]],
    ['-ment', '-mento', [['moment', 'momento'], ['document', 'documento'], ['element', 'elemento']]],
    ['ph-', 'f-', [['photo', 'foto'], ['telephone', 'teléfono'], ['philosophy', 'filosofía']]],
    ['st- / sp-', 'est- / esp-', [['student', 'estudiante'], ['special', 'especial'], ['station', 'estación'], ['stress', 'estrés']]],
    ['-ize', '-izar', [['organize', 'organizar'], ['analyze', 'analizar'], ['memorize', 'memorizar']]],
    ['-ate (verb)', '-ar', [['celebrate', 'celebrar'], ['communicate', 'comunicar'], ['participate', 'participar']]]
  ];
  C.falseFriends = [
    ['embarazada', 'pregnant', 'embarrassed', 'avergonzado / tengo vergüenza'], ['éxito', 'success', 'exit', 'salida'],
    ['librería', 'bookstore', 'library', 'biblioteca'], ['actual', 'current', 'actual', 'real'],
    ['actualmente', 'currently', 'actually', 'en realidad'], ['realizar', 'to carry out', 'to realize', 'darse cuenta'],
    ['asistir', 'to attend', 'to assist', 'ayudar'], ['carpeta', 'folder', 'carpet', 'alfombra'],
    ['constipado', 'having a cold', 'constipated', 'estreñido'], ['molestar', 'to bother', 'to molest', 'acosar'],
    ['sensible', 'sensitive', 'sensible', 'sensato'], ['recordar', 'to remember', 'to record', 'grabar'],
    ['largo', 'long', 'large', 'grande'], ['ropa', 'clothes', 'rope', 'cuerda'],
    ['fábrica', 'factory', 'fabric', 'tela'], ['soportar', 'to put up with', 'to support', 'apoyar'],
    ['pretender', 'to intend, try', 'to pretend', 'fingir'], ['decepción', 'disappointment', 'deception', 'engaño'],
    ['introducir', 'to insert', 'to introduce (a person)', 'presentar'], ['compromiso', 'commitment', 'compromise', 'acuerdo']
  ];

  // ---- Trick 5 · Gender: always learn el/la. [word with article, english, note when it breaks the rule] --
  C.genderRules = [
    ['-o → usually el', 'el libro, el carro', 'Exceptions: la mano, la foto, la moto, la radio'],
    ['-a → usually la', 'la casa, la mesa', 'Exceptions: el día, el mapa, el planeta, el sofá'],
    ['-ma (Greek origin) → el', 'el problema, el sistema, el tema, el idioma, el programa, el clima', ''],
    ['-ción, -sión, -dad, -tad, -tud, -umbre → la', 'la canción, la ciudad, la actitud, la costumbre', 'Always feminine'],
    ['-aje, -or → el', 'el viaje, el garaje, el color, el amor', 'Exception: la flor'],
    ['Stressed a- at the start → el, but still feminine', 'el agua fría, el águila', 'Plural goes back to las: las aguas']
  ];
  C.genderWords = [
    ['el mapa', 'map', 'exception'], ['el día', 'day', 'exception'], ['el problema', 'problem', '-ma'], ['el sistema', 'system', '-ma'], ['el tema', 'topic', '-ma'],
    ['el idioma', 'language', '-ma'], ['el programa', 'program', '-ma'], ['el clima', 'climate', '-ma'], ['el planeta', 'planet', 'exception'], ['el sofá', 'sofa', 'exception'],
    ['la mano', 'hand', 'exception'], ['la foto', 'photo', 'exception'], ['la moto', 'motorcycle', 'exception'], ['la radio', 'radio', 'exception'],
    ['el agua', 'water (feminine!)', 'el + stressed a'], ['la gente', 'people', ''], ['la noche', 'night', ''], ['la clase', 'class', ''], ['la leche', 'milk', ''],
    ['el coche', 'car (Spain)', ''], ['la calle', 'street', ''], ['la parte', 'part', ''], ['el viaje', 'trip', '-aje'], ['el garaje', 'garage', '-aje'],
    ['la ciudad', 'city', '-dad'], ['la libertad', 'freedom', '-tad'], ['la canción', 'song', '-ción'], ['la costumbre', 'custom, habit', '-umbre'], ['la actitud', 'attitude', '-tud'],
    ['el lápiz', 'pencil', ''], ['la luz', 'light', ''], ['la nariz', 'nose', ''], ['el amor', 'love', '-or'], ['la flor', 'flower', 'exception'],
    ['el corazón', 'heart', ''], ['la razón', 'reason', ''], ['la imagen', 'image', ''], ['el examen', 'exam', ''], ['el árbol', 'tree', ''],
    ['la sal', 'salt', ''], ['el sol', 'sun', ''], ['el hotel', 'hotel', ''], ['la pared', 'wall', ''], ['el pie', 'foot', ''], ['la piel', 'skin', ''],
    ['el café', 'coffee', ''], ['la llave', 'key', ''], ['el avión', 'airplane', ''], ['la televisión', 'television', '-sión'], ['la verdad', 'truth', '-dad'], ['el color', 'color', '-or']
  ];

  // ---- Trick 4 · Pronunciation (train your mouth) --------------------------------------------------------
  C.pronunciation = [
    { title: 'Pure vowels: A E I O U', tip: 'Spanish vowels are short and never glide. "No" is not "noh-oo"; "me" is not "may".', items: ['casa, mamá, papá', 'mesa, leche, verde', 'sí, mira, libro', 'todo, poco, ojo', 'uno, luna, fruta', 'La mamá de Ana está en casa.'] },
    { title: 'Tap R vs. trilled RR', tip: 'One R between vowels is a quick tap (like the "tt" in American "butter"). RR — and an R at the start of a word — is a trill.', items: ['pero — perro', 'caro — carro', 'cero — cerro', 'pera — perra', 'rojo, rápido, Roma', 'El perro de Rosa corre rápido.'] },
    { title: 'Soft D, B = V, silent H', tip: 'D between vowels is soft, close to the "th" in "the". B and V sound the same. H is always silent.', items: ['nada, todo, cansado', 'vamos, bueno, vino', 'hola, hablar, hora', 'Todo está bien, ¿verdad?'] },
    { title: 'J, G, LL, Ñ', tip: 'J (and G before e/i) is a breathy "h" from the throat. In Latin America LL sounds like Y. Ñ is like "ny" in "canyon".', items: ['jugo, gente, jirafa', 'llamar, calle, yo', 'año, mañana, España', 'Mañana voy a la playa con Julia.'] }
  ];
  C.tongueTwisters = [
    'Tres tristes tigres tragaban trigo en un trigal.',
    'Erre con erre cigarro, erre con erre barril. Rápido ruedan los carros cargados de azúcar del ferrocarril.',
    'Pablito clavó un clavito. ¿Qué clavito clavó Pablito?',
    'Como poco coco como, poco coco compro.',
    'El perro de San Roque no tiene rabo porque Ramón Ramírez se lo ha cortado.',
    'Pancha plancha con cuatro planchas. ¿Con cuántas planchas plancha Pancha?'
  ];

  // ---- Trick 2 · Language island: describe it in Spanish (circumlocution) ---------------------------------
  C.describeFrames = ['Es una cosa que…', 'Es un lugar donde…', 'Es una persona que…', 'Sirve para…', 'Se usa cuando…', 'Es parecido a…, pero…', 'Es lo contrario de…'];
  C.describe = [
    ['a ladder', 'la escalera', 'Es una cosa que usas para subir a un lugar alto.'], ['a stapler', 'la engrapadora (Spain: grapadora)', 'Es un objeto de oficina que sirve para unir papeles.'],
    ['a wallet', 'la billetera / la cartera', 'Es una cosa pequeña donde pones el dinero y las tarjetas.'], ['an umbrella', 'el paraguas', 'Es un objeto que usas cuando llueve para no mojarte.'],
    ['a pharmacist', 'el farmacéutico / la farmacéutica', 'Es una persona que trabaja en la farmacia y vende medicinas.'], ['a sink', 'el lavabo / el lavamanos', 'Es el lugar del baño donde te lavas las manos.'],
    ['to borrow', 'pedir prestado', 'Es cuando tomas algo de otra persona y después lo devuelves.'], ['a landlord', 'el casero / el dueño', 'Es la persona que es dueña de tu apartamento y te cobra la renta.'],
    ['a hangover', 'la cruda (MX) / la resaca', 'Es como te sientes la mañana después de beber mucho alcohol.'], ['a blender', 'la licuadora', 'Es una máquina de la cocina que sirve para hacer jugos y batidos.'],
    ['a receipt', 'el recibo / el ticket', 'Es el papel que te dan en la tienda después de pagar.'], ['a flat tire', 'una llanta ponchada (MX) / un pinchazo', 'Es cuando una rueda del carro no tiene aire.'],
    ['to wait in line', 'hacer fila / hacer cola', 'Es cuando esperas detrás de otras personas para pagar o entrar.'], ['a nephew', 'el sobrino', 'Es el hijo de mi hermano o de mi hermana.'],
    ['a lighthouse', 'el faro', 'Es una torre en la costa con una luz para los barcos.'], ['glue', 'el pegamento', 'Es una cosa que sirve para unir papel u otros objetos.'],
    ['to sneeze', 'estornudar', 'Es lo que haces con la nariz cuando tienes alergia o estás resfriado.'], ['a stroller', 'la carriola (MX) / el cochecito', 'Es una cosa con ruedas para llevar a un bebé.'],
    ['a dishwasher', 'el lavaplatos / el lavavajillas', 'Es una máquina que lava los platos.'], ['to iron', 'planchar', 'Es lo que haces con la ropa para quitar las arrugas.']
  ];

  // ---- Trick 3 · Comprehensible input + Trick 2 · devices + Trick 4 · exchanges ----------------------------
  const yt = q => 'https://www.youtube.com/results?search_query=' + encodeURIComponent(q);
  C.resources = [
    { name: 'Dreaming Spanish', level: 'A1 → B2', accent: 'Spain + Latin America', kind: 'Video', url: 'https://www.dreamingspanish.com', note: 'Comprehensible input sorted by level. Start with Intermediate videos; their roadmap measures progress in hours of listening.' },
    { name: 'No Hay Tos (Real Mexican Spanish)', level: 'A2+ → B2', accent: 'Mexico', kind: 'Podcast', url: 'https://podcasts.apple.com/podcast/id1360162037', note: 'Native speakers at natural speed, with slang explained. Excellent for the Mexican accent — try 0.9× speed.' },
    { name: 'Español con Juan', level: 'B1 → B2', accent: 'Spain', kind: 'Video / podcast', url: yt('Español con Juan'), note: 'Short, funny lessons told slowly in Spanish only. Great once you reach phase 3.' },
    { name: 'Duolingo Spanish Podcast', level: 'A2 → B1', accent: 'Mixed', kind: 'Podcast', url: 'https://podcast.duolingo.com', note: 'True stories in easy Spanish with English narration between parts. A gentle bridge to native audio.' },
    { name: 'Easy Spanish (street interviews)', level: 'B1', accent: 'Mexico, Spain, Latin America', kind: 'Video', url: yt('Easy Spanish street interviews'), note: 'Real people on the street. Switch the subtitles to Spanish only.' }
  ];
  // Series made in Spanish, by the level they suit: a learner sees their own level first, and the next one opens
  // as their checkpoints show the level (progress.js seriesLevel). Availability is Spain's catalogue as of
  // November 2026; catalogues change and differ by country. All of these are from Spain, with the Castilian accent
  // and vosotros, which the course doesn't teach: extra listening, not a model to copy.
  C.series = [
    { name: 'Extra en español', level: 'A1', years: '2002', kind: 'Sitcom made for learners',
      note: 'Sam, an American, moves in with three flatmates in Madrid. Slow, clear Spanish and short episodes written for beginners.',
      where: [['YouTube', 'free']] },
    { name: 'Aquí no hay quien viva', level: 'A2', years: '2003–2006', kind: 'Sitcom',
      note: 'The neighbours of one Madrid building and one everyday problem per episode. The same characters and phrases come back again and again, which is what an A2 ear needs. Use Spanish subtitles.',
      where: [['Netflix', 'complete'], ['Prime Video', 'complete'], ['Max', 'complete'], ['Atresplayer', ''], ['Disney+', '']] },
    { name: 'Machos alfa', level: 'B1', years: '2022–', kind: 'Comedy',
      note: 'Four friends in their forties trying to keep up with a world that changed. Everyday Spanish of today at natural speed, with slang.',
      where: [['Netflix', 'complete, Netflix only']] },
    { name: 'Con el culo al aire', level: 'B1', years: '2012–2014', kind: 'Comedy',
      note: 'People who have lost almost everything end up living at a campsite. Lots of characters and colloquial speech.',
      where: [['Atresplayer', 'free with ads']] },
    { name: 'La que se avecina', level: 'B1', years: '2007–', kind: 'Sitcom',
      note: 'The neighbours of a housing estate outside Madrid: fast, full of slang and catchphrases. Leave it for last; it is hard even at B1.',
      where: [['Prime Video', 'complete'], ['Disney+', ''], ['Mitele', '']] }
  ];
  C.seriesWhere = 'Where to watch in Spain, November 2026. Catalogues change and differ by country.';
  C.seriesTips = [
    'Pick a series you already know well (Friends, The Office…) so the plot never blocks you.',
    'Audio: Español (Latinoamérica). Subtitles: Español — never English.',
    'If it is too fast, slow it to 0.9× or re-watch a scene twice.',
    'Pause and repeat one line per scene out loud (shadowing).'
  ];
  C.missions = [
    { id: 'phone', title: 'iPhone in Spanish', how: 'Settings → General → Language & Region → iPhone Language → Español (México).' },
    { id: 'mac', title: 'Mac apps in Spanish', how: 'System Settings → General → Language & Region → Applications → "+" → pick Safari or Chrome → Español.' },
    { id: 'social', title: 'Social media in Spanish', how: 'Instagram / TikTok / Facebook → Settings → Language → Español.' },
    { id: 'youtube', title: 'YouTube & Netflix in Spanish', how: 'Change the account language to Español and set Spanish subtitles by default.' },
    { id: 'siri', title: 'Siri in Spanish', how: 'Settings → Siri → Language → Español (Estados Unidos o México). Ask Siri the weather in Spanish every morning.' },
    { id: 'keyboard', title: 'Spanish keyboard', how: 'iPhone: Settings → General → Keyboard → Add Español. On the Mac, hold a vowel to type á é í ó ú, or press Option+E then the vowel.' },
    { id: 'app', title: 'LearnQuick in Spanish', how: 'Switch this app\'s interface to Spanish (button below or in Settings).' }
  ];
  C.exchangePhrases = [
    ['¿Hablamos media hora en español y media hora en inglés?', 'Shall we do half an hour in Spanish and half in English?'],
    ['¿Me puedes corregir cuando me equivoque?', 'Can you correct me when I make a mistake?'],
    ['¿Cómo se dice … en español?', 'How do you say … in Spanish?'],
    ['¿Lo puedes repetir más despacio?', 'Can you repeat it more slowly?'],
    ['No entendí. ¿Qué significa …?', "I didn't understand. What does … mean?"],
    ['¿Lo dije bien?', 'Did I say it right?']
  ];

  // Shown on the Dashboard, one per day.
  C.tricks = [
    { n: 1, title: 'The 80/20 verbs', text: 'Master ser, estar, tener, hacer, poder, querer, ir and decir in present, past and ir a + infinitive. They carry most conversations.', route: 'practice/verbs' },
    { n: 1, title: 'Cognates are free words', text: '-tion → -ción, -ty → -dad, -ous → -oso. You already know thousands of Spanish words.', route: 'practice/cognates' },
    { n: 2, title: 'Switch your devices', text: 'Put your phone and apps in Spanish: dozens of tiny reading sessions every day, for free.', route: 'immersion' },
    { n: 2, title: 'Language island', text: 'One hour a day with zero English. If you do not know a word, describe it in Spanish.', route: 'practice/island' },
    { n: 3, title: 'Comprehensible input', text: 'Listen to A2–B1 content you mostly understand. Dreaming Spanish, Español con Juan or No Hay Tos at 0.9×.', route: 'immersion' },
    { n: 3, title: 'Spanish subtitles only', text: 'Watch a series you know, dubbed in Spanish, with Spanish subtitles — never English.', route: 'immersion' },
    { n: 4, title: 'Train your mouth', text: 'Read aloud. Exaggerate the R and keep vowels short and pure — your speaking muscles need reps.', route: 'practice/speak' },
    { n: 4, title: 'Language exchanges', text: 'On Tandem or HelloTalk, Spanish speakers will correct you in exchange for English practice.', route: 'immersion' },
    { n: 5, title: 'Fui vs. iba is the filter', text: 'Do not obsess over the subjunctive. Narrating the past correctly is what makes you B1.', route: 'practice/past' },
    { n: 5, title: 'Learn el / la with every noun', text: 'Never learn "mapa" — learn "el mapa". Gender sticks when the article comes with it.', route: 'practice/gender' }
  ];

  // ---- Placement test: 15 items, A1 → B1 -----------------------------------------------------------------
  C.placement = [
    { q: '¿Cómo te llamas?', o: ['Me llamo Ana.', 'Tengo Ana.', 'Soy llamo Ana.'] },
    { q: 'Yo ___ de Estados Unidos.', o: ['soy', 'estoy', 'tengo'] },
    { q: 'Nosotros ___ dos hijos.', o: ['tenemos', 'somos', 'estamos'] },
    { q: '"¿Dónde está el baño?" means…', o: ['Where is the bathroom?', 'When is the bathroom?', 'What is the bathroom?'] },
    { q: 'Me ___ mucho las películas de terror.', o: ['gustan', 'gusta', 'gusto'] },
    { q: 'Ayer yo ___ al cine con mis amigos.', o: ['fui', 'voy', 'iba'] },
    { q: 'Mañana ___ a visitar a mi abuela.', o: ['voy', 'fui', 'iba'] },
    { q: 'Cuando era niño, ___ en un pueblo pequeño.', o: ['vivía', 'viví', 'vivo'] },
    { q: '¿Me ___ traer la cuenta, por favor?', o: ['puede', 'pueda', 'poder'] },
    { q: 'Estudio español desde ___ dos años.', o: ['hace', 'por', 'hasta'] },
    { q: 'Caminaba por la calle cuando ___ un accidente.', o: ['vi', 'veía', 'he visto'] },
    { q: '¿Alguna vez ___ en Perú?', o: ['has estado', 'estás', 'estarás'] },
    { q: 'Si tengo tiempo, te ___ mañana.', o: ['llamaré', 'llamaría', 'llamé'] },
    { q: 'Quiero que tú ___ a la fiesta.', o: ['vengas', 'vienes', 'venir'] },
    { q: 'Yo que tú, ___ con el médico.', o: ['hablaría', 'hablo', 'hablaba'] }
  ];
  C.placementLevel = score => score <= 6 ? { level: 'A1', plan: 90 } : score <= 11 ? { level: 'A2', plan: 60 } : { level: 'A2+ / B1−', plan: 30 };

  // ---- B1 mock (final checkpoint), modelled on the four DELE B1 skills ------------------------------------
  C.mock = {
    reading: {
      text: 'Hola, Marta:\n\n¿Cómo estás? Te escribo desde Guadalajara. Llegué hace dos semanas y todavía me estoy acostumbrando a la ciudad. Al principio fue difícil porque no conocía a nadie y mi español no era muy bueno. Sin embargo, ahora tengo una rutina: por las mañanas trabajo en una cafetería cerca de mi apartamento y por las tardes tomo clases de cocina mexicana. ¡Ya sé preparar mole!\n\nEl fin de semana pasado fui a Tequila con unos compañeros de clase. Mientras visitábamos una fábrica, empezó a llover muchísimo y tuvimos que esperar dos horas. Fue divertido de todos modos.\n\nEl mes que viene voy a mudarme a un apartamento más grande, aunque será un poco más caro. Me encantaría recibirte en diciembre. ¿Puedes venir?\n\nUn abrazo,\nDaniel',
      questions: [
        { q: '¿Cuánto tiempo lleva Daniel en Guadalajara?', o: ['Dos semanas', 'Dos meses', 'Un año'] },
        { q: '¿Por qué fue difícil al principio?', o: ['No conocía a nadie y su español no era bueno.', 'No le gustaba la comida.', 'Hacía mucho calor.'] },
        { q: '¿Qué hace Daniel por las tardes?', o: ['Toma clases de cocina.', 'Trabaja en una cafetería.', 'Visita fábricas.'] },
        { q: '¿Qué pasó en Tequila?', o: ['Llovió y tuvieron que esperar.', 'Perdieron el autobús.', 'La fábrica estaba cerrada.'] },
        { q: '¿Qué va a hacer el mes que viene?', o: ['Mudarse a un apartamento más grande.', 'Volver a Estados Unidos.', 'Visitar a Marta.'] }
      ]
    },
    listening: {
      text: 'Hola, soy Carmen, de la clínica dental Sonrisa. Le llamo para confirmar su cita del jueves a las cuatro y media. Por favor, llegue diez minutos antes y traiga su tarjeta del seguro. Si no puede venir, llámenos antes del miércoles. Ah, y recuerde que nos mudamos: ahora estamos en la calle Hidalgo, número veinte, al lado del banco. ¡Que tenga un buen día!',
      questions: [
        { q: '¿Cuándo es la cita?', o: ['El jueves a las 4:30', 'El miércoles a las 4:30', 'El jueves a las 10:00'] },
        { q: '¿Qué tiene que traer?', o: ['Su tarjeta del seguro', 'Su pasaporte', 'Dinero en efectivo'] },
        { q: '¿Cuándo debe llegar?', o: ['Diez minutos antes', 'A las cuatro en punto', 'Media hora antes'] },
        { q: '¿Qué cambió en la clínica?', o: ['La dirección', 'El nombre de la dentista', 'El horario'] },
        { q: '¿Dónde está la clínica ahora?', o: ['Al lado del banco', 'Enfrente del banco', 'Cerca del hospital'] }
      ]
    },
    writing: { prompt: 'Escribe un correo a un amigo (100–150 palabras). Cuéntale un viaje o una experiencia reciente: qué pasó, cómo era el lugar, cómo te sentiste y qué planes tienes para el futuro.' },
    speaking: [
      { title: 'Presentación (2–3 min)', prompt: 'Habla de tu trabajo o tus estudios y de tu experiencia aprendiendo español: cuándo empezaste, qué fue difícil y qué te ayudó.' },
      { title: 'Conversación', prompt: 'Responde en voz alta: ¿Qué es lo más difícil del español para ti? ¿Por qué? ¿Qué consejo le darías a otra persona que empieza?' },
      { title: 'Describe la imagen', prompt: 'Describe la ilustración de la cafetería: ¿qué hay?, ¿cómo es el lugar?, ¿qué crees que pasó antes y qué va a pasar después?', image: 'cafe' },
      { title: 'Situación', prompt: 'Estás en un hotel y el aire acondicionado no funciona. Explica el problema en recepción y pide una solución con cortesía.' }
    ]
  };
  if (typeof module !== 'undefined') module.exports = LQ;
})(typeof window !== 'undefined' ? window : globalThis);
