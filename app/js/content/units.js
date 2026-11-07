/* LearnQuick course: 25 units in 5 phases, A2 → B1. Neutral Latin-American Spanish (ustedes), with Spain
   variants noted. Conventions: vocab = [spanish, english, example]; dialogue = [speaker, spanish, english]
   ('Tú' is the learner); in 'mc' exercises the FIRST option is correct — options are shuffled at runtime.
   Content written for this app; have a Spanish teacher review it before wider use. */
(function (root) {
  'use strict';
  const LQ = root.LQ = root.LQ || {};
  const C = LQ.content = LQ.content || {};

  C.phases = [
    { n: 1, en: 'Foundations 80/20', es: 'Fundamentos 80/20', icon: 'book', desc: 'Key verbs, ser/estar, plans with ir a, cognates and gender.' },
    { n: 2, en: 'Everyday Conversations', es: 'Conversaciones diarias', icon: 'waveform', desc: 'Questions, likes, routine, restaurants and directions.' },
    { n: 3, en: 'Telling the Past', es: 'Contar el pasado', icon: 'clock', desc: 'Pretérito, imperfecto and fui vs. iba — the real B1 filter.' },
    { n: 4, en: 'Plans, Opinions & Politeness', es: 'Planes, opiniones y cortesía', icon: 'headphones', desc: 'Experiences, future, conditional, comparing and por/para.' },
    { n: 5, en: 'Real-World B1', es: 'Español B1 real', icon: 'award', desc: 'Subjunctive (recognition), health, work, travel problems, your story.' }
  ];

  const RAW = [
    // ===================================== PHASE 1 · FOUNDATIONS 80/20 =====================================
    {
      title: 'Ser & Estar: Who and How You Are', es: 'Ser y estar', canDo: 'Introduce yourself and say how and where you are.', trick: 1,
      vocab: [['estadounidense', 'American (from the US)', 'Soy estadounidense, de Chicago.'], ['el trabajo', 'work, job', 'Mi trabajo es interesante.'], ['la oficina', 'office', 'Estoy en la oficina.'], ['cansado / cansada', 'tired', 'Hoy estoy muy cansado.'], ['contento / contenta', 'happy, glad', 'Estoy contenta con mi clase.'], ['ocupado / ocupada', 'busy', 'Esta semana estoy ocupado.'], ['simpático / simpática', 'nice, friendly', 'Mi jefe es simpático.'], ['alto / alta', 'tall', 'Mi hermano es alto.'], ['la ciudad', 'city', 'Austin es una ciudad grande.'], ['casado / casada', 'married', 'Estoy casado.'], ['ahora', 'now', 'Ahora estoy en casa.'], ['la casa', 'house, home', 'Mi casa está cerca.']],
      grammar: {
        title: 'Ser vs. Estar — the 80/20 rule',
        hack: 'Both mean "to be". Use SER for what something IS (identity, origin, profession, personality, time). Use ESTAR for HOW or WHERE something is right now (feelings, conditions, location).',
        tables: [{ head: ['', 'ser', 'estar'], rows: [['yo', 'soy', 'estoy'], ['tú', 'eres', 'estás'], ['él / ella / usted', 'es', 'está'], ['nosotros', 'somos', 'estamos'], ['ellos / ustedes', 'son', 'están']] }],
        examples: [['Soy ingeniero.', 'I am an engineer. (profession → ser)'], ['Soy de Ohio.', 'I am from Ohio. (origin → ser)'], ['Estoy cansado.', 'I am tired. (condition → estar)'], ['La oficina está en el centro.', 'The office is downtown. (location → estar)']],
        tip: 'Location of EVENTS uses ser: "La fiesta es en mi casa." Location of people and things uses estar.'
      },
      dialogue: [['Lucía', '¡Hola! Soy Lucía. ¿Y tú?', "Hi! I'm Lucía. And you?"], ['Tú', 'Mucho gusto. Soy de Chicago, pero ahora vivo en Austin.', "Nice to meet you. I'm from Chicago, but now I live in Austin."], ['Lucía', '¡Qué bien! ¿Y a qué te dedicas?', 'Great! And what do you do?'], ['Tú', 'Soy ingeniero. Trabajo en una empresa de tecnología.', "I'm an engineer. I work at a tech company."], ['Lucía', '¿Y estás contento en Austin?', 'And are you happy in Austin?'], ['Tú', 'Sí, muy contento. Pero hoy estoy un poco cansado.', "Yes, very happy. But today I'm a little tired."], ['Lucía', '¡Normal, es viernes! La fiesta es en mi casa a las ocho.', "Of course, it's Friday! The party is at my house at eight."], ['Tú', '¡Perfecto! ¿Dónde está tu casa?', 'Perfect! Where is your house?']],
      ex: [
        { t: 'mc', q: 'Mi hermana ___ doctora.', o: ['es', 'está'], why: 'Profession → ser.' },
        { t: 'mc', q: '"Estamos en el aeropuerto." What does it mean?', o: ['We are at the airport.', 'We are from the airport.', 'We work at the airport.'] },
        { t: 'fill', q: 'Yo ___ muy cansado hoy.', a: 'estoy', hint: 'estar', why: 'A temporary condition → estar.' },
        { t: 'fill', q: 'Nosotros ___ de Estados Unidos.', a: 'somos', hint: 'ser', why: 'Origin → ser.' },
        { t: 'fill', q: '¿Dónde ___ la oficina?', a: 'está', hint: 'estar', why: 'Location of a place → estar.' },
        { t: 'mc', q: '"The party is at my house."', o: ['La fiesta es en mi casa.', 'La fiesta está en mi casa.'], why: 'Events "take place" → ser.' },
        { t: 'order', es: 'Mi jefe es muy simpático', en: 'My boss is very nice' },
        { t: 'listen', es: 'Ahora estoy en la oficina', en: 'Now I am at the office' },
        { t: 'fill', q: 'Ellos ___ casados.', a: 'están', hint: 'estar', why: 'Marital status normally uses estar: estar casado.' }
      ],
      speak: ['Soy de Chicago, pero ahora vivo en Austin.', 'Hoy estoy un poco cansado.', '¿Dónde está tu casa?'],
      coach: 'Spanish vowels are short and pure. Say "es-TOY", not "es-TOY-ee". Keep each vowel clean.'
    },
    {
      title: 'Tener, Querer, Poder: Needs & Wants', es: 'Tener, querer y poder', canDo: 'Say what you have, want, need and can do.', trick: 1,
      vocab: [['tener hambre', 'to be hungry', 'Tengo mucha hambre.'], ['tener sed', 'to be thirsty', '¿Tienes sed?'], ['tener prisa', 'to be in a hurry', 'Lo siento, tengo prisa.'], ['tener que + infinitivo', 'to have to (do)', 'Tengo que trabajar.'], ['la reunión', 'meeting', 'Tengo una reunión a las tres.'], ['el tiempo', 'time; weather', 'No tengo tiempo hoy.'], ['la semana', 'week', 'Trabajo cinco días a la semana.'], ['el fin de semana', 'weekend', '¿Qué haces el fin de semana?'], ['el dinero', 'money', 'No tengo dinero en efectivo.'], ['hoy', 'today', 'Hoy no puedo.'], ['mañana', 'tomorrow; morning', 'Mañana tengo tiempo.'], ['también', 'also, too', 'Yo también quiero ir.']],
      grammar: {
        title: 'Three verbs, hundreds of sentences',
        hack: 'TENGO (I have), QUIERO (I want), PUEDO (I can). Add an infinitive after querer, poder or tener que and you are done: Quiero comer. Puedo ir. Tengo que trabajar.',
        tables: [{ head: ['', 'tener', 'querer', 'poder'], rows: [['yo', 'tengo', 'quiero', 'puedo'], ['tú', 'tienes', 'quieres', 'puedes'], ['él / ella / usted', 'tiene', 'quiere', 'puede'], ['nosotros', 'tenemos', 'queremos', 'podemos'], ['ellos / ustedes', 'tienen', 'quieren', 'pueden']] }],
        examples: [['Tengo 35 años.', 'I am 35 years old. (literally: I have 35 years)'], ['¿Quieres un café?', 'Do you want a coffee?'], ['No puedo hoy porque tengo una reunión.', "I can't today because I have a meeting."], ['Tenemos que salir a las siete.', 'We have to leave at seven.']],
        tip: 'They are "boot verbs": the stem changes (e→ie, o→ue) in every form except nosotros. Draw a boot around the forms that change.'
      },
      dialogue: [['Carlos', '¿Tienes tiempo para un café?', 'Do you have time for a coffee?'], ['Tú', 'Hoy no puedo. Tengo una reunión a las tres.', "I can't today. I have a meeting at three."], ['Carlos', '¿Y mañana? ¿Quieres comer juntos?', 'And tomorrow? Do you want to have lunch together?'], ['Tú', 'Sí, quiero. Pero tengo que salir a las dos.', 'Yes, I want to. But I have to leave at two.'], ['Carlos', 'Perfecto. ¿Puedes a la una?', 'Perfect. Can you make it at one?'], ['Tú', 'Sí, puedo. A esa hora siempre tengo hambre.', "Yes, I can. I'm always hungry at that time."], ['Carlos', '¡Yo también! Hasta mañana.', 'Me too! See you tomorrow.']],
      ex: [
        { t: 'fill', q: 'Yo ___ que trabajar el sábado.', a: 'tengo', hint: 'tener' },
        { t: 'fill', q: '¿Tú ___ ir al cine esta noche?', a: 'quieres', hint: 'querer' },
        { t: 'fill', q: 'Nosotros no ___ salir hoy.', a: 'podemos', hint: 'poder', why: 'Nosotros keeps the original stem: podemos.' },
        { t: 'mc', q: '"Tengo prisa" means…', o: ['I am in a hurry.', 'I am hungry.', 'I have a meeting.'] },
        { t: 'mc', q: 'How do you say "I am 30 years old"?', o: ['Tengo 30 años.', 'Soy 30 años.', 'Estoy 30 años.'], why: 'Age uses tener.' },
        { t: 'order', es: 'No puedo porque tengo una reunión', en: "I can't because I have a meeting" },
        { t: 'listen', es: '¿Quieres comer juntos mañana?', en: 'Do you want to eat together tomorrow?' },
        { t: 'fill', q: 'Ellos ___ mucha sed.', a: 'tienen', hint: 'tener' },
        { t: 'mc', q: 'Which sentence is correct?', o: ['Ella puede venir.', 'Ella pode venir.', 'Ella poder venir.'] }
      ],
      speak: ['Hoy no puedo, tengo una reunión.', '¿Quieres tomar un café mañana?', 'Tengo que salir a las dos.'],
      coach: 'Use "tener que + infinitive" for obligations. It is the most common way to say "have to" in conversation.'
    },
    {
      title: 'Present Tense: Your Week', es: 'El presente: tu semana', canDo: 'Describe your routine and what you usually do.', trick: 1,
      vocab: [['trabajar', 'to work', 'Trabajo de lunes a viernes.'], ['vivir', 'to live', 'Vivo en un apartamento.'], ['comer', 'to eat', 'Como a la una.'], ['el gimnasio', 'gym', 'Voy al gimnasio temprano.'], ['el supermercado', 'supermarket', 'El supermercado está cerca.'], ['siempre', 'always', 'Siempre desayuno café.'], ['a veces', 'sometimes', 'A veces como tarde.'], ['nunca', 'never', 'Nunca trabajo los domingos.'], ['temprano', 'early', 'Me gusta salir temprano.'], ['tarde', 'late; afternoon', 'Llego tarde a casa.'], ['los lunes', 'on Mondays', 'Los lunes voy al gimnasio.'], ['la verdad', 'the truth', 'Siempre digo la verdad.']],
      grammar: {
        title: 'Three endings + three irregular stars',
        hack: 'Regular verbs: drop -ar / -er / -ir and add the endings. Then learn the three superstar irregulars: hacer (hago), ir (voy), decir (digo).',
        tables: [
          { head: ['', 'hablar', 'comer', 'vivir'], rows: [['yo', 'hablo', 'como', 'vivo'], ['tú', 'hablas', 'comes', 'vives'], ['él / ella / usted', 'habla', 'come', 'vive'], ['nosotros', 'hablamos', 'comemos', 'vivimos'], ['ellos / ustedes', 'hablan', 'comen', 'viven']] },
          { head: ['', 'hacer', 'ir', 'decir'], rows: [['yo', 'hago', 'voy', 'digo'], ['tú', 'haces', 'vas', 'dices'], ['él / ella / usted', 'hace', 'va', 'dice'], ['nosotros', 'hacemos', 'vamos', 'decimos'], ['ellos / ustedes', 'hacen', 'van', 'dicen']] }
        ],
        examples: [['Hago ejercicio los lunes.', 'I exercise on Mondays.'], ['Voy al supermercado los sábados.', 'I go to the supermarket on Saturdays.'], ['Siempre digo la verdad.', 'I always tell the truth.'], ['¿Qué haces los fines de semana?', 'What do you do on weekends?']],
        tip: 'Learn every verb in the "yo" form first — it is the one you use most when you talk about yourself.'
      },
      dialogue: [['Ana', '¿Qué haces normalmente los sábados?', 'What do you usually do on Saturdays?'], ['Tú', 'Por la mañana voy al gimnasio y por la tarde hago las compras.', 'In the morning I go to the gym and in the afternoon I do the shopping.'], ['Ana', '¿Vas al gimnasio temprano?', 'Do you go to the gym early?'], ['Tú', 'Sí, siempre voy a las siete. ¿Y tú?', 'Yes, I always go at seven. And you?'], ['Ana', 'Yo nunca hago ejercicio los sábados. Leo, cocino y descanso.', 'I never exercise on Saturdays. I read, cook and rest.'], ['Tú', 'Mis amigos dicen que trabajo demasiado.', 'My friends say I work too much.'], ['Ana', '¡Tienen razón! Los sábados son para descansar.', "They're right! Saturdays are for resting."]],
      ex: [
        { t: 'fill', q: 'Yo ___ ejercicio los lunes.', a: 'hago', hint: 'hacer' },
        { t: 'fill', q: 'Nosotros ___ al supermercado los sábados.', a: 'vamos', hint: 'ir' },
        { t: 'fill', q: 'Ella siempre ___ la verdad.', a: 'dice', hint: 'decir' },
        { t: 'fill', q: 'Ustedes ___ en Texas, ¿verdad?', a: 'viven', hint: 'vivir' },
        { t: 'mc', q: '"A veces como tarde." means…', o: ['Sometimes I eat late.', 'I never eat late.', 'I always eat in the afternoon.'] },
        { t: 'mc', q: 'Los lunes, yo ___ al gimnasio.', o: ['voy', 'vo', 'va'] },
        { t: 'order', es: 'Siempre voy al gimnasio temprano', en: 'I always go to the gym early' },
        { t: 'listen', es: '¿Qué haces los fines de semana?', en: 'What do you do on weekends?' },
        { t: 'fill', q: 'Tú ___ mucho, ¿no?', a: 'trabajas', hint: 'trabajar' }
      ],
      speak: ['Los lunes voy al gimnasio.', '¿Qué haces los fines de semana?', 'Mis amigos dicen que trabajo demasiado.'],
      coach: 'Narrate what you do while you do it: "Ahora hago café…". It is free speaking practice from day one.'
    },
    {
      title: 'Ir a + Infinitive: Your Plans', es: 'Ir a + infinitivo: planes', canDo: 'Talk about your plans for the weekend and the near future.', trick: 1,
      vocab: [['la próxima semana', 'next week', 'La próxima semana voy a viajar.'], ['este fin de semana', 'this weekend', 'Este fin de semana voy a descansar.'], ['esta noche', 'tonight', '¿Qué vas a hacer esta noche?'], ['pronto', 'soon', 'Vamos a hablar pronto.'], ['viajar', 'to travel', 'Me encanta viajar.'], ['visitar', 'to visit', 'Voy a visitar a mis padres.'], ['descansar', 'to rest', 'El domingo voy a descansar.'], ['la playa', 'beach', 'Vamos a ir a la playa.'], ['el cumpleaños', 'birthday', 'Mi cumpleaños es en mayo.'], ['la familia', 'family', 'Mi familia vive en Ohio.'], ['el plan', 'plan', '¿Tienes planes para el sábado?'], ['juntos / juntas', 'together', 'Vamos a comer juntos.']],
      grammar: {
        title: 'The easiest future: ir + a + infinitive',
        hack: 'Conjugate IR, add A, add any infinitive. Voy a viajar = I am going to travel. You can talk about the future from day one without learning a new tense.',
        tables: [{ head: ['', 'ir', '+ a + infinitive'], rows: [['yo', 'voy a', 'trabajar'], ['tú', 'vas a', 'descansar'], ['él / ella / usted', 'va a', 'viajar'], ['nosotros', 'vamos a', 'comer'], ['ellos / ustedes', 'van a', 'salir']] }],
        examples: [['Voy a visitar a mi familia.', 'I am going to visit my family.'], ['¿Qué vas a hacer esta noche?', 'What are you going to do tonight?'], ['Vamos a ir a la playa.', 'We are going to go to the beach.'], ['No voy a trabajar el sábado.', 'I am not going to work on Saturday.']],
        tip: '"¡Vamos a + infinitive!" also means "Let\'s…": ¡Vamos a comer! = Let\'s eat! And use "a" before people: visitar a mi familia.'
      },
      dialogue: [['Sofía', '¿Qué vas a hacer este fin de semana?', 'What are you going to do this weekend?'], ['Tú', 'Voy a descansar. El sábado voy a ver una película.', "I'm going to rest. On Saturday I'm going to watch a movie."], ['Sofía', 'Nosotros vamos a ir a la playa. ¿Quieres venir?', "We're going to go to the beach. Do you want to come?"], ['Tú', '¿En serio? ¿A qué hora van a salir?', 'Really? What time are you going to leave?'], ['Sofía', 'Vamos a salir temprano, a las ocho.', "We're going to leave early, at eight."], ['Tú', 'Perfecto. Voy a llevar algo de comer.', "Perfect. I'm going to bring something to eat."], ['Sofía', '¡Genial! Va a ser un día fantástico.', "Great! It's going to be a fantastic day."]],
      ex: [
        { t: 'fill', q: 'Yo ___ a viajar en diciembre.', a: 'voy', hint: 'ir' },
        { t: 'fill', q: '¿Qué ___ a hacer ustedes esta noche?', a: 'van', hint: 'ir' },
        { t: 'mc', q: '"Vamos a comer" can mean…', o: ["Let's eat / We are going to eat", 'We ate', 'We eat every day'] },
        { t: 'fill', q: 'Mañana nosotros ___ a descansar.', a: 'vamos', hint: 'ir' },
        { t: 'order', es: 'Voy a visitar a mi familia', en: 'I am going to visit my family' },
        { t: 'mc', q: 'Choose the correct sentence:', o: ['Ella va a trabajar mañana.', 'Ella va trabajar mañana.', 'Ella va a trabaja mañana.'], why: 'ir + a + infinitive (trabajar).' },
        { t: 'listen', es: '¿A qué hora van a salir?', en: 'What time are you going to leave?' },
        { t: 'fill', q: 'No ___ a trabajar el domingo.', a: 'voy', hint: 'ir, yo' },
        { t: 'mc', q: '"Va a ser un día fantástico." means…', o: ['It is going to be a fantastic day.', 'It was a fantastic day.', 'It is a fantastic day.'] }
      ],
      speak: ['Este fin de semana voy a descansar.', '¿Qué vas a hacer esta noche?', 'Vamos a salir temprano.'],
      coach: 'With ir a + infinitive you already sound B1 when you talk about plans. Practise it about your real week.'
    },
    {
      title: 'Cognates & Gender: Instant Vocabulary', es: 'Cognados y género', canDo: 'Turn English words into Spanish and always learn nouns with el/la.', trick: 1,
      vocab: [['la celebración', 'celebration', 'La celebración es el sábado.'], ['la comunidad', 'community', 'Es una comunidad muy activa.'], ['la oportunidad', 'opportunity', 'Es una gran oportunidad.'], ['la universidad', 'university', 'Estudié en la universidad de Texas.'], ['el problema', 'problem (masculine!)', 'No hay problema.'], ['el sistema', 'system (masculine!)', 'El sistema no funciona.'], ['el mapa', 'map (masculine!)', '¿Tienes un mapa de la ciudad?'], ['el día', 'day (masculine!)', 'Buenos días. ¡Qué buen día!'], ['la mano', 'hand (feminine!)', 'Me duele la mano.'], ['la foto', 'photo (feminine: la fotografía)', 'Mira esta foto.'], ['el programa', 'program, show (masculine!)', 'Es mi programa favorito.'], ['realmente', 'really', 'Es realmente fácil.']],
      grammar: {
        title: 'Cognate rules + el/la',
        hack: 'Thousands of English words become Spanish with a simple ending change. And every noun has a gender: learn it with its article, never alone.',
        tables: [{ head: ['English', 'Spanish', 'Example'], rows: [['-tion', '-ción', 'celebration → celebración'], ['-ty', '-dad', 'community → comunidad'], ['-ous', '-oso', 'famous → famoso'], ['-ly', '-mente', 'really → realmente'], ['-ist', '-ista', 'artist → artista'], ['-ic / -ical', '-ico', 'typical → típico'], ['-ive', '-ivo', 'creative → creativo'], ['-ence / -ance', '-encia / -ancia', 'patience → paciencia'], ['-ary', '-ario', 'necessary → necesario']] }],
        examples: [['La celebración es en la universidad.', 'The celebration is at the university.'], ['Es una oportunidad fantástica.', 'It is a fantastic opportunity.'], ['El problema es el sistema.', 'The problem is the system.'], ['Tengo una foto del mapa.', 'I have a photo of the map.']],
        tip: 'Words ending in -ción, -dad and -tud are always feminine. Many words ending in -ma are masculine (el problema, el sistema, el programa). Beware false friends: "embarazada" means pregnant, not embarrassed!'
      },
      dialogue: [['Jefa', 'Necesito información sobre el problema del sistema.', 'I need information about the system problem.'], ['Tú', 'Claro. Realmente es una situación complicada.', "Sure. It's really a complicated situation."], ['Jefa', '¿Hay una solución posible?', 'Is there a possible solution?'], ['Tú', 'Sí. Es necesario actualizar el programa.', "Yes. It's necessary to update the program."], ['Jefa', 'Perfecto. Es una oportunidad para la comunidad de usuarios.', "Perfect. It's an opportunity for the user community."], ['Tú', 'Exacto. Y la celebración del lanzamiento es el día quince.', 'Exactly. And the launch celebration is on the fifteenth.']],
      ex: [
        { t: 'mc', q: '"Celebration" in Spanish is…', o: ['la celebración', 'el celebración', 'la celebrasión'], why: '-tion → -ción, and -ción words are feminine.' },
        { t: 'fill', q: 'English "community" = la ___', a: 'comunidad', hint: '-ty → -dad' },
        { t: 'fill', q: 'English "famous" = ___', a: 'famoso', hint: '-ous → -oso' },
        { t: 'mc', q: 'Which is correct?', o: ['el problema', 'la problema'], why: 'Greek-origin -ma words are masculine.' },
        { t: 'mc', q: 'Which is correct?', o: ['la mano', 'el mano'], why: 'Mano is the famous exception: feminine despite the -o.' },
        { t: 'mc', q: '"Estoy embarazada" means…', o: ['I am pregnant.', 'I am embarrassed.', 'I am busy.'], why: 'False friend! "I am embarrassed" = Tengo vergüenza.' },
        { t: 'fill', q: 'English "really" = ___', a: 'realmente', hint: '-ly → -mente' },
        { t: 'order', es: 'Es una oportunidad fantástica', en: 'It is a fantastic opportunity' },
        { t: 'listen', es: 'El problema es el sistema', en: 'The problem is the system' },
        { t: 'mc', q: 'Which is correct?', o: ['el día', 'la día'], why: 'Día is masculine despite the -a.' }
      ],
      speak: ['Es una oportunidad fantástica.', 'La celebración es el día quince.', 'Realmente es una situación complicada.'],
      coach: 'Make cognates sound Spanish: stress the ending and keep vowels pure — ce-le-bra-CIÓN, co-mu-ni-DAD.'
    },
    // ===================================== PHASE 2 · EVERYDAY CONVERSATIONS =================================
    {
      title: 'Questions, Numbers & Shopping', es: 'Preguntas, números y compras', canDo: 'Ask questions, understand prices and buy things.', trick: 4, art: 'cafe',
      vocab: [['¿cuánto cuesta?', 'how much does it cost?', '¿Cuánto cuesta esta camisa?'], ['¿dónde?', 'where?', '¿Dónde está la tienda?'], ['¿cuándo?', 'when?', '¿Cuándo abren?'], ['¿por qué?', 'why?', '¿Por qué no vienes?'], ['porque', 'because', 'No voy porque estoy cansado.'], ['barato / barata', 'cheap', 'Esta chaqueta es barata.'], ['caro / cara', 'expensive', 'Es un poco caro.'], ['la tienda', 'store, shop', 'La tienda abre a las diez.'], ['la talla', 'size (clothes)', '¿Cuál es su talla?'], ['la tarjeta', 'card', '¿Puedo pagar con tarjeta?'], ['el efectivo', 'cash', 'Solo tengo efectivo.'], ['cien', 'one hundred', 'Cuesta cien dólares.']],
      grammar: {
        title: 'Question words + big numbers',
        hack: 'Question words always carry an accent: qué, cuál, dónde, cuándo, cuánto, cómo, quién, por qué. The answer "porque" (because) is one word with no accent.',
        tables: [
          { head: ['Question word', 'Meaning', 'Example'], rows: [['qué', 'what', '¿Qué buscas?'], ['cuál', 'which / what', '¿Cuál es tu talla?'], ['dónde', 'where', '¿Dónde está la tienda?'], ['cuándo', 'when', '¿Cuándo abren?'], ['cuánto', 'how much', '¿Cuánto cuesta?'], ['cuántos', 'how many', '¿Cuántos quieres?'], ['cómo', 'how', '¿Cómo pagas?'], ['quién', 'who', '¿Quién es?']] },
          { head: ['Number', 'Spanish'], rows: [['50', 'cincuenta'], ['100', 'cien'], ['150', 'ciento cincuenta'], ['200', 'doscientos'], ['500', 'quinientos'], ['1.000', 'mil'], ['2026', 'dos mil veintiséis']] }
        ],
        examples: [['¿Cuánto cuesta esta camisa?', 'How much does this shirt cost?'], ['Cuesta cuarenta y cinco dólares.', 'It costs forty-five dollars.'], ['¿Puedo pagar con tarjeta?', 'Can I pay by card?'], ['¿Cuál es tu número de teléfono?', 'What is your phone number?']],
        tip: 'Use "¿Cuál es…?" (not "¿Qué es…?") to ask for specific information: ¿Cuál es tu dirección? = What is your address?'
      },
      dialogue: [['Vendedora', 'Buenas tardes. ¿Qué busca?', 'Good afternoon. What are you looking for?'], ['Tú', 'Busco una chaqueta. ¿Cuánto cuesta esta?', "I'm looking for a jacket. How much is this one?"], ['Vendedora', 'Cuesta ciento veinte dólares.', 'It costs one hundred twenty dollars.'], ['Tú', 'Uf, es un poco cara. ¿Tiene algo más barato?', "Oof, it's a bit expensive. Do you have something cheaper?"], ['Vendedora', 'Sí, esta cuesta ochenta. ¿Cuál es su talla?', 'Yes, this one is eighty. What is your size?'], ['Tú', 'Mediana. ¿Puedo pagar con tarjeta?', 'Medium. Can I pay by card?'], ['Vendedora', 'Claro, con tarjeta o en efectivo.', 'Of course, by card or in cash.']],
      ex: [
        { t: 'mc', q: '"¿Dónde está la tienda?" asks…', o: ['where the store is', 'when the store opens', 'why the store is closed'] },
        { t: 'fill', q: '¿___ cuesta el café?', a: 'cuánto', hint: 'how much' },
        { t: 'fill', q: '¿___ es tu talla?', a: 'cuál', hint: 'which / what' },
        { t: 'mc', q: 'How do you say 500?', o: ['quinientos', 'cincocientos', 'quiniento'], why: 'Irregular: quinientos (500), setecientos (700), novecientos (900).' },
        { t: 'mc', q: 'No compro la chaqueta ___ es cara.', o: ['porque', 'por qué', 'porqué'] },
        { t: 'listen', es: '¿Puedo pagar con tarjeta?', en: 'Can I pay by card?' },
        { t: 'order', es: '¿Tiene algo más barato?', en: 'Do you have something cheaper?' },
        { t: 'mc', q: '150 =', o: ['ciento cincuenta', 'cien cincuenta', 'uno cincuenta'], why: '"Cien" alone = 100; 101–199 use "ciento".' },
        { t: 'fill', q: '¿___ abren mañana? — A las nueve.', a: 'cuándo', hint: 'when' }
      ],
      speak: ['¿Cuánto cuesta esta chaqueta?', '¿Tiene algo más barato?', '¿Puedo pagar con tarjeta?'],
      coach: 'Useful in any shop: "¿Me lo puedo probar?" = Can I try it on?'
    },
    {
      title: 'Gustar & Friends: Likes and Dislikes', es: 'Gustar y compañía', canDo: 'Talk about your likes, dislikes and interests.', trick: 4,
      vocab: [['me gusta', 'I like (one thing / action)', 'Me gusta el café.'], ['me gustan', 'I like (plural things)', 'Me gustan los tacos.'], ['me encanta', 'I love (it)', 'Me encanta viajar.'], ['me interesa', "I'm interested in", 'Me interesa la historia.'], ['me molesta', 'it bothers me', 'Me molesta el ruido.'], ['el fútbol', 'soccer', 'No me gusta el fútbol.'], ['la música', 'music', 'Me encanta la música latina.'], ['la película', 'movie', 'Es una película muy buena.'], ['leer', 'to read', 'Me gusta leer por la noche.'], ['cocinar', 'to cook', '¿Te gusta cocinar?'], ['la comida picante', 'spicy food', 'Me encanta la comida picante.'], ['el ruido', 'noise', 'Hay mucho ruido aquí.']],
      grammar: {
        title: 'Gustar works backwards',
        hack: 'Spanish says "music pleases me": Me gusta la música. The verb agrees with the THING you like: gusta (one thing or an action), gustan (plural things).',
        tables: [{ head: ['Who likes it', '+ gusta (1 thing / action)', '+ gustan (plural)'], rows: [['(a mí) me', 'me gusta', 'me gustan'], ['(a ti) te', 'te gusta', 'te gustan'], ['(a él / ella / usted) le', 'le gusta', 'le gustan'], ['(a nosotros) nos', 'nos gusta', 'nos gustan'], ['(a ellos / ustedes) les', 'les gusta', 'les gustan']] }],
        examples: [['Me gusta cocinar.', 'I like to cook.'], ['Me encantan las películas de acción.', 'I love action movies.'], ['A mi hermano no le gusta el fútbol.', "My brother doesn't like soccer."], ['¿Te interesa la historia?', 'Are you interested in history?']],
        tip: 'Same pattern: encantar, interesar, molestar, importar, doler. Answer "me too" with "A mí también" and "me neither" with "A mí tampoco".'
      },
      dialogue: [['Diego', '¿Qué te gusta hacer en tu tiempo libre?', 'What do you like to do in your free time?'], ['Tú', 'Me encanta cocinar y me gustan mucho las películas.', 'I love cooking and I really like movies.'], ['Diego', '¿Te gusta la comida picante?', 'Do you like spicy food?'], ['Tú', 'Sí, me encanta. ¿Y a ti?', 'Yes, I love it. And you?'], ['Diego', 'A mí no me gusta mucho. Me interesa más la música.', "I don't like it much. I'm more interested in music."], ['Tú', 'A mí también me gusta la música, pero me molesta el ruido de los conciertos.', 'I like music too, but the noise at concerts bothers me.'], ['Diego', '¡Ja! Entonces, ¿un concierto acústico?', 'Ha! So, an acoustic concert?']],
      ex: [
        { t: 'mc', q: '"Me gustan los tacos." Why "gustan"?', o: ['Because "los tacos" is plural.', 'Because "me" is plural.', 'Because it is past tense.'] },
        { t: 'fill', q: 'Me ___ la música latina.', a: 'gusta', hint: 'gustar' },
        { t: 'fill', q: 'A nosotros nos ___ las películas.', a: 'gustan', hint: 'gustar' },
        { t: 'fill', q: 'A ella no ___ gusta el fútbol.', a: 'le', hint: 'pronoun' },
        { t: 'mc', q: 'How do you say "I love cooking"?', o: ['Me encanta cocinar.', 'Yo encanto cocinar.', 'Me encantan cocinar.'], why: 'Actions (infinitives) take the singular: me encanta + infinitive.' },
        { t: 'mc', q: '"¿Te molesta el ruido?" means…', o: ['Does the noise bother you?', 'Do you like noise?', 'Is the noise loud?'] },
        { t: 'order', es: 'A mi hermano no le gusta el fútbol', en: "My brother doesn't like soccer" },
        { t: 'listen', es: '¿Qué te gusta hacer en tu tiempo libre?', en: 'What do you like to do in your free time?' },
        { t: 'fill', q: '¿A ustedes ___ gusta bailar?', a: 'les', hint: 'pronoun' }
      ],
      speak: ['Me encanta cocinar.', '¿Qué te gusta hacer en tu tiempo libre?', 'A mí también me gusta la música.'],
      coach: 'Turn every like into a mini-conversation: "Me encanta… ¿Y a ti?" Asking back keeps exchanges going.'
    },
    {
      title: 'Daily Routine: Reflexive Verbs', es: 'La rutina: verbos reflexivos', canDo: 'Describe your daily routine from morning to night.', trick: 4,
      vocab: [['despertarse', 'to wake up', 'Me despierto a las seis.'], ['levantarse', 'to get up', 'Me levanto temprano.'], ['ducharse', 'to shower', 'Me ducho por la mañana.'], ['vestirse', 'to get dressed', 'Me visto rápido.'], ['acostarse', 'to go to bed', 'Me acuesto a las once.'], ['sentirse', 'to feel', 'Hoy me siento bien.'], ['quedarse', 'to stay', 'Me quedo en casa el domingo.'], ['el desayuno', 'breakfast', 'El desayuno es a las siete.'], ['la cena', 'dinner', 'La cena está lista.'], ['primero', 'first', 'Primero me ducho.'], ['luego', 'then, later', 'Luego desayuno.'], ['después', 'after, afterwards', 'Después voy a trabajar.']],
      grammar: {
        title: 'Reflexive verbs: me, te, se',
        hack: 'Many routine verbs are reflexive: the action comes back to you. Put the pronoun before the conjugated verb: me levanto, te duchas, se acuesta.',
        tables: [{ head: ['', 'levantarse', 'acostarse (o→ue)'], rows: [['yo', 'me levanto', 'me acuesto'], ['tú', 'te levantas', 'te acuestas'], ['él / ella / usted', 'se levanta', 'se acuesta'], ['nosotros', 'nos levantamos', 'nos acostamos'], ['ellos / ustedes', 'se levantan', 'se acuestan']] }],
        examples: [['Me levanto a las seis y media.', 'I get up at six thirty.'], ['Primero me ducho y luego desayuno.', 'First I shower and then I have breakfast.'], ['¿A qué hora te acuestas?', 'What time do you go to bed?'], ['Hoy me siento muy bien.', 'Today I feel very good.']],
        tip: 'With an infinitive, attach the pronoun to the end: Voy a levantarme temprano (= Me voy a levantar temprano).'
      },
      dialogue: [['Marta', 'Siempre estás cansado. ¿A qué hora te levantas?', "You're always tired. What time do you get up?"], ['Tú', 'Me levanto a las cinco y media para ir al gimnasio.', 'I get up at five thirty to go to the gym.'], ['Marta', '¿Y a qué hora te acuestas?', 'And what time do you go to bed?'], ['Tú', 'Normalmente me acuesto a medianoche.', 'I usually go to bed at midnight.'], ['Marta', '¡Solo duermes cinco horas y media! ¿Cómo te sientes?', 'You only sleep five and a half hours! How do you feel?'], ['Tú', 'La verdad, me siento muy cansado por la tarde.', 'Honestly, I feel very tired in the afternoon.'], ['Marta', 'Tienes que acostarte más temprano.', 'You have to go to bed earlier.']],
      ex: [
        { t: 'fill', q: 'Yo ___ levanto a las siete.', a: 'me', hint: 'pronoun' },
        { t: 'fill', q: 'Ellos se ___ a las once.', a: 'acuestan', hint: 'acostarse' },
        { t: 'fill', q: '¿A qué hora ___ duchas?', a: 'te', hint: 'pronoun' },
        { t: 'mc', q: '"Me siento bien" means…', o: ['I feel good.', 'I sit down well.', 'I am sitting.'], why: 'Sentirse = to feel. (Sentarse = to sit down — careful!)' },
        { t: 'order', es: 'Primero me ducho y luego desayuno', en: 'First I shower and then I have breakfast' },
        { t: 'mc', q: 'Mañana voy a ___ temprano.', o: ['levantarme', 'me levantar', 'levanto'] },
        { t: 'listen', es: '¿A qué hora te acuestas normalmente?', en: 'What time do you usually go to bed?' },
        { t: 'fill', q: 'Nosotros nos ___ en casa los domingos.', a: 'quedamos', hint: 'quedarse' },
        { t: 'mc', q: 'Which verb means "to get dressed"?', o: ['vestirse', 'despertarse', 'quedarse'] }
      ],
      speak: ['Me levanto a las seis y media.', 'Primero me ducho y luego desayuno.', 'Normalmente me acuesto a las once.'],
      coach: 'Describe your morning out loud in Spanish while you get ready. Two minutes a day adds up fast.'
    },
    {
      title: 'At the Restaurant', es: 'En el restaurante', canDo: 'Order food politely and handle the check.', trick: 4, art: 'cafe',
      vocab: [['el mesero / la mesera', 'waiter / waitress (Spain: camarero)', 'El mesero es muy amable.'], ['la cuenta', 'the check, the bill', 'La cuenta, por favor.'], ['el menú', 'menu', '¿Me trae el menú?'], ['la mesa', 'table', 'Una mesa para dos.'], ['la bebida', 'drink', '¿Qué bebida quiere?'], ['el plato', 'dish, plate', 'Este plato es delicioso.'], ['el postre', 'dessert', '¿Hay postre?'], ['el agua', 'water (feminine, but "el agua")', 'Un agua sin gas, por favor.'], ['la propina', 'tip', 'La propina no está incluida.'], ['quisiera', 'I would like (polite)', 'Quisiera la sopa del día.'], ['para llevar', 'to go', 'Un café para llevar.'], ['rico / rica', 'delicious (food)', '¡Qué rica está la comida!']],
      grammar: {
        title: 'Polite requests + lo / la / los / las',
        hack: 'Order with: Quisiera…, ¿Me trae…?, ¿Me puede traer…?, Para mí… To avoid repeating a noun, replace it with lo / la / los / las before the verb.',
        tables: [{ head: ['Thing', 'Pronoun', 'Example'], rows: [['el café', 'lo', 'Lo quiero con leche.'], ['la cuenta', 'la', '¿Me la trae, por favor?'], ['los tacos', 'los', 'Los pido para llevar.'], ['las papas', 'las', 'Las quiero sin sal.']] }],
        examples: [['¿Me puede traer un café, por favor?', 'Can you bring me a coffee, please?'], ['Para mí, la sopa del día.', 'For me, the soup of the day.'], ['Quisiera una mesa para dos.', 'I would like a table for two.'], ['La cuenta, por favor.', 'The check, please.']],
        tip: '"¿Me la trae?" = Can you bring it (the check) to me? The person (me) goes first, then the thing (la).'
      },
      dialogue: [['Mesero', 'Buenas noches. ¿Mesa para cuántos?', 'Good evening. A table for how many?'], ['Tú', 'Para dos, por favor. ¿Me puede traer el menú?', 'For two, please. Can you bring me the menu?'], ['Mesero', 'Claro. ¿Qué van a tomar?', 'Of course. What will you have to drink?'], ['Tú', 'Para mí, un agua sin gas. Y quisiera los tacos al pastor.', "For me, still water. And I'd like the tacos al pastor."], ['Mesero', 'Muy bien. ¿Los quiere con cebolla y cilantro?', 'Very good. Do you want them with onion and cilantro?'], ['Tú', 'Sí, con todo. Aquí están muy ricos.', "Yes, with everything. They're really good here."], ['Mesero', '¿Algo de postre?', 'Any dessert?'], ['Tú', 'No, gracias. La cuenta, por favor.', 'No, thank you. The check, please.']],
      ex: [
        { t: 'mc', q: 'The most polite way to ask for a table:', o: ['Quisiera una mesa para dos.', 'Quiero mesa.', 'Dame una mesa.'] },
        { t: 'fill', q: '¿El café? ___ quiero con leche.', a: 'lo', hint: 'el café → ?' },
        { t: 'fill', q: '¿La cuenta? ¿Me ___ trae, por favor?', a: 'la', hint: 'la cuenta → ?' },
        { t: 'fill', q: '¿Los tacos? ___ pido para llevar.', a: 'los', hint: 'los tacos → ?' },
        { t: 'mc', q: '"Para llevar" means…', o: ['to go', 'for the table', 'to bring'] },
        { t: 'mc', q: 'Why "el agua" if agua is feminine?', o: ['A stressed "a-" at the start takes "el" for sound.', 'Agua is masculine.', 'It is a mistake.'], why: 'It stays feminine: el agua fría.' },
        { t: 'order', es: 'Quisiera una mesa para dos', en: 'I would like a table for two' },
        { t: 'listen', es: '¿Me puede traer la cuenta, por favor?', en: 'Can you bring me the check, please?' },
        { t: 'fill', q: '¿Las papas? ___ quiero sin sal.', a: 'las', hint: 'las papas → ?' }
      ],
      speak: ['¿Me puede traer el menú, por favor?', 'Para mí, un agua sin gas.', 'La cuenta, por favor.'],
      coach: 'In Mexico you can call the waiter with "Disculpe" or "Joven". In Spain, "Perdone".'
    },
    {
      title: 'The City: Asking for Directions', es: 'La ciudad: direcciones', canDo: 'Ask for and give directions in a city.', trick: 4,
      vocab: [['la calle', 'street', 'Vivo en esta calle.'], ['la esquina', 'corner', 'Está en la esquina.'], ['el semáforo', 'traffic light', 'Gire en el semáforo.'], ['la cuadra', 'block (Spain: la manzana)', 'Está a dos cuadras.'], ['a la derecha', 'to the right', 'Gire a la derecha.'], ['a la izquierda', 'to the left', 'El banco está a la izquierda.'], ['derecho', 'straight ahead (Spain: recto)', 'Siga derecho.'], ['cerca (de)', 'near', 'El hotel está cerca.'], ['lejos (de)', 'far (from)', '¿Está lejos de aquí?'], ['la estación', 'station', 'La estación de metro está ahí.'], ['el metro', 'subway', 'Voy en metro.'], ['al lado de', 'next to', 'Está al lado del banco.']],
      grammar: {
        title: 'Hay vs. está + commands for directions',
        hack: 'Use HAY to say something exists (Is there a pharmacy?) and ESTÁ for the location of a specific place (Where is the pharmacy?). For directions, use usted commands: siga, gire, tome, cruce.',
        tables: [{ head: ['Infinitive', 'Usted (formal)', 'Tú (informal)'], rows: [['seguir', 'siga', 'sigue'], ['girar', 'gire', 'gira'], ['tomar', 'tome', 'toma'], ['cruzar', 'cruce', 'cruza'], ['caminar', 'camine', 'camina'], ['ir', 'vaya', 've']] }],
        examples: [['¿Hay una farmacia cerca?', 'Is there a pharmacy nearby?'], ['¿Dónde está la estación de metro?', 'Where is the subway station?'], ['Siga derecho dos cuadras y gire a la derecha.', 'Go straight two blocks and turn right.'], ['Está al lado del banco.', 'It is next to the bank.']],
        tip: 'Usted commands swap the vowel: -ar verbs end in -e (gire), -er/-ir verbs in -a (siga). It is the same form you will meet later in the subjunctive.'
      },
      dialogue: [['Tú', 'Disculpe, ¿hay una farmacia por aquí?', 'Excuse me, is there a pharmacy around here?'], ['Señora', 'Sí, hay una muy cerca. Siga derecho por esta calle.', 'Yes, there is one very close. Go straight along this street.'], ['Tú', '¿Cuántas cuadras?', 'How many blocks?'], ['Señora', 'Dos cuadras. En el semáforo, gire a la izquierda.', 'Two blocks. At the traffic light, turn left.'], ['Tú', '¿Y está lejos del metro?', 'And is it far from the subway?'], ['Señora', 'No, está al lado de la estación. No se puede perder.', "No, it's next to the station. You can't miss it."], ['Tú', 'Muchas gracias, muy amable.', 'Thank you very much, very kind.']],
      ex: [
        { t: 'mc', q: '"Is there a bank nearby?"', o: ['¿Hay un banco cerca?', '¿Está un banco cerca?', '¿Es un banco cerca?'], why: 'Existence → hay.' },
        { t: 'mc', q: '"Where is the museum?"', o: ['¿Dónde está el museo?', '¿Dónde hay el museo?'], why: 'A specific place → está.' },
        { t: 'fill', q: '___ derecho dos cuadras.', a: 'siga', hint: 'seguir, usted' },
        { t: 'fill', q: 'En la esquina, ___ a la derecha.', a: 'gire', hint: 'girar, usted' },
        { t: 'mc', q: '"A la izquierda" means…', o: ['to the left', 'to the right', 'straight ahead'] },
        { t: 'order', es: 'Siga derecho y gire a la izquierda', en: 'Go straight and turn left' },
        { t: 'listen', es: 'La estación está al lado del banco', en: 'The station is next to the bank' },
        { t: 'fill', q: '___ la calle en el semáforo.', a: 'cruce', hint: 'cruzar, usted' },
        { t: 'mc', q: '"Está lejos" means…', o: ['It is far.', 'It is near.', 'It is late.'] }
      ],
      speak: ['Disculpe, ¿hay una farmacia por aquí?', '¿Dónde está la estación de metro?', 'Siga derecho y gire a la derecha.'],
      coach: 'If you don\'t understand, ask: "¿Me lo puede repetir más despacio, por favor?" — Could you repeat it more slowly?'
    },
    // ===================================== PHASE 3 · TELLING THE PAST ======================================
    {
      title: 'Preterite: What You Did Yesterday', es: 'El pretérito: ¿qué hiciste ayer?', canDo: 'Say what you did yesterday and last week.', trick: 5,
      vocab: [['ayer', 'yesterday', 'Ayer trabajé mucho.'], ['anoche', 'last night', 'Anoche cené tarde.'], ['la semana pasada', 'last week', 'La semana pasada viajé a Miami.'], ['el año pasado', 'last year', 'El año pasado empecé a estudiar español.'], ['hace dos días', 'two days ago', 'Llegué hace dos días.'], ['llegar', 'to arrive', 'Llegué a las nueve.'], ['terminar', 'to finish', 'Terminé el proyecto.'], ['llamar', 'to call', 'Te llamé ayer.'], ['salir', 'to go out, leave', 'Salimos el sábado.'], ['escribir', 'to write', 'Escribí un correo.'], ['aprender', 'to learn', 'Aprendí mucho en la clase.'], ['conocer', 'to meet (someone); to know', 'Conocí a Laura en Madrid.']],
      grammar: {
        title: 'Pretérito: finished actions',
        hack: 'Use the pretérito for completed actions at a specific time: ayer, anoche, el lunes, hace dos días. Regular endings are short, and the yo and él forms carry an accent.',
        tables: [{ head: ['', 'hablar', 'comer', 'vivir'], rows: [['yo', 'hablé', 'comí', 'viví'], ['tú', 'hablaste', 'comiste', 'viviste'], ['él / ella / usted', 'habló', 'comió', 'vivió'], ['nosotros', 'hablamos', 'comimos', 'vivimos'], ['ellos / ustedes', 'hablaron', 'comieron', 'vivieron']] }],
        examples: [['Ayer trabajé diez horas.', 'Yesterday I worked ten hours.'], ['Anoche comimos en un restaurante.', 'Last night we ate at a restaurant.'], ['¿Llamaste a tu mamá?', 'Did you call your mom?'], ['Salí de la oficina a las seis.', 'I left the office at six.']],
        tip: 'The accent changes the meaning: hablo = I speak; habló = he/she spoke. Spelling changes in the yo form: llegué, busqué, empecé.'
      },
      dialogue: [['Pablo', '¿Qué tal tu fin de semana?', 'How was your weekend?'], ['Tú', 'Muy bien. El sábado salí con unos amigos y comimos en un restaurante peruano.', 'Very good. On Saturday I went out with some friends and we ate at a Peruvian restaurant.'], ['Pablo', '¡Qué rico! ¿Y el domingo?', 'Yum! And on Sunday?'], ['Tú', 'El domingo descansé y llamé a mi familia. ¿Y tú?', 'On Sunday I rested and called my family. And you?'], ['Pablo', 'Yo trabajé todo el sábado. Terminé el proyecto anoche.', 'I worked all Saturday. I finished the project last night.'], ['Tú', '¡Felicidades! ¿A qué hora llegaste a casa?', 'Congratulations! What time did you get home?'], ['Pablo', 'Llegué a las once. Estoy muerto.', "I got home at eleven. I'm exhausted."]],
      ex: [
        { t: 'fill', q: 'Ayer yo ___ diez horas.', a: 'trabajé', hint: 'trabajar' },
        { t: 'fill', q: 'Anoche nosotros ___ pizza.', a: 'comimos', hint: 'comer' },
        { t: 'fill', q: '¿Tú ___ a tu mamá?', a: 'llamaste', hint: 'llamar' },
        { t: 'fill', q: 'Ella ___ un correo largo.', a: 'escribió', hint: 'escribir' },
        { t: 'mc', q: '"Hablo" vs. "habló":', o: ['hablo = I speak; habló = he/she spoke', 'They mean the same.', 'hablo = I spoke; habló = he speaks'] },
        { t: 'mc', q: 'Yo ___ a las ocho. (llegar)', o: ['llegué', 'llegé', 'llegó'], why: 'Spelling: g → gu before e keeps the hard sound.' },
        { t: 'order', es: 'El sábado salí con unos amigos', en: 'On Saturday I went out with some friends' },
        { t: 'listen', es: '¿A qué hora llegaste a casa?', en: 'What time did you get home?' },
        { t: 'fill', q: 'Ellos ___ mucho en la clase.', a: 'aprendieron', hint: 'aprender' }
      ],
      speak: ['El sábado salí con unos amigos.', 'Ayer trabajé diez horas.', '¿A qué hora llegaste a casa?'],
      coach: 'Every evening, say three sentences about your day in the pretérito. Short, real, daily.'
    },
    {
      title: 'Irregular Preterite: Fui, Tuve, Hice', es: 'Pretérito irregular', canDo: 'Tell someone about a trip or a special day.', trick: 5,
      vocab: [['el viaje', 'trip', 'El viaje fue increíble.'], ['el vuelo', 'flight', 'El vuelo salió tarde.'], ['el hotel', 'hotel', 'El hotel estaba en el centro.'], ['las vacaciones', 'vacation', 'Fui de vacaciones a México.'], ['el lugar', 'place', 'Es un lugar muy bonito.'], ['la gente', 'people (singular in Spanish!)', 'La gente fue muy amable.'], ['increíble', 'incredible', '¡Fue una experiencia increíble!'], ['divertido / divertida', 'fun', 'La fiesta fue muy divertida.'], ['el museo', 'museum', 'Visitamos un museo.'], ['la montaña', 'mountain', 'Subimos la montaña.'], ['perder', 'to lose; to miss (a flight)', 'Perdimos el vuelo.'], ['el pasaporte', 'passport', '¿Dónde está mi pasaporte?']],
      grammar: {
        title: 'Superstar irregulars in the past',
        hack: 'The most frequent verbs are irregular in the pretérito, but they follow patterns and have NO accents: tuve, estuve, pude, puse, supe, quise, hice, vine, dije. Ser and ir share the same forms: fui, fue, fuimos.',
        tables: [
          { head: ['', 'ser / ir', 'tener', 'hacer', 'estar'], rows: [['yo', 'fui', 'tuve', 'hice', 'estuve'], ['tú', 'fuiste', 'tuviste', 'hiciste', 'estuviste'], ['él / ella / usted', 'fue', 'tuvo', 'hizo', 'estuvo'], ['nosotros', 'fuimos', 'tuvimos', 'hicimos', 'estuvimos'], ['ellos / ustedes', 'fueron', 'tuvieron', 'hicieron', 'estuvieron']] },
          { head: ['', 'poder', 'querer', 'decir', 'venir'], rows: [['yo', 'pude', 'quise', 'dije', 'vine'], ['tú', 'pudiste', 'quisiste', 'dijiste', 'viniste'], ['él / ella / usted', 'pudo', 'quiso', 'dijo', 'vino'], ['nosotros', 'pudimos', 'quisimos', 'dijimos', 'vinimos'], ['ellos / ustedes', 'pudieron', 'quisieron', 'dijeron', 'vinieron']] }
        ],
        examples: [['Fui a México en diciembre.', 'I went to Mexico in December.'], ['Fue un viaje increíble.', 'It was an incredible trip.'], ['Tuvimos un problema con el vuelo.', 'We had a problem with the flight.'], ['¿Qué hiciste el fin de semana?', 'What did you do on the weekend?']],
        tip: 'Fui = I went OR I was. Context decides: "Fui a Cancún" (went) vs. "Fui muy feliz allí" (was).'
      },
      dialogue: [['Elena', '¿Qué hiciste en tus vacaciones?', 'What did you do on your vacation?'], ['Tú', 'Fui a Oaxaca con mi familia. Estuvimos allí una semana.', 'I went to Oaxaca with my family. We were there for a week.'], ['Elena', '¡Qué bien! ¿Y qué tal fue?', 'How nice! And how was it?'], ['Tú', 'Fue increíble. La gente fue muy amable y la comida… ¡riquísima!', 'It was incredible. The people were very kind and the food… delicious!'], ['Elena', '¿Tuvieron algún problema?', 'Did you have any problems?'], ['Tú', 'Sí, perdimos el vuelo de regreso y tuvimos que quedarnos una noche más.', 'Yes, we missed the flight back and had to stay one more night.'], ['Elena', 'Bueno, ¡no es un problema terrible!', "Well, that's not a terrible problem!"]],
      ex: [
        { t: 'fill', q: 'El año pasado yo ___ a España.', a: 'fui', hint: 'ir' },
        { t: 'fill', q: 'El viaje ___ increíble.', a: 'fue', hint: 'ser' },
        { t: 'fill', q: 'Nosotros ___ un problema con el hotel.', a: 'tuvimos', hint: 'tener' },
        { t: 'fill', q: '¿Qué ___ tú el sábado?', a: 'hiciste', hint: 'hacer' },
        { t: 'fill', q: 'Ellos ___ en la playa todo el día.', a: 'estuvieron', hint: 'estar' },
        { t: 'mc', q: '"Ella no pudo venir." means…', o: ["She couldn't come.", "She can't come.", "She didn't want to come."] },
        { t: 'mc', q: 'Which form is correct?', o: ['hizo', 'hació', 'hiso'], why: 'hacer → hizo (the z keeps the sound).' },
        { t: 'order', es: 'Fue un viaje increíble', en: 'It was an incredible trip' },
        { t: 'listen', es: '¿Qué hiciste en tus vacaciones?', en: 'What did you do on your vacation?' },
        { t: 'fill', q: 'Mi jefe me ___ que sí.', a: 'dijo', hint: 'decir' }
      ],
      speak: ['Fui a Oaxaca con mi familia.', 'Fue un viaje increíble.', 'Tuvimos que quedarnos una noche más.'],
      coach: 'Memorize "fui, fue, fuimos" as a chunk. You will use it in almost every story you tell.'
    },
    {
      title: 'Imperfect: When I Was a Kid', es: 'El imperfecto: cuando era niño', canDo: 'Describe your childhood and past habits.', trick: 5,
      vocab: [['de niño / de niña', 'as a child', 'De niño vivía en Ohio.'], ['cuando era joven', 'when I was young', 'Cuando era joven, jugaba al béisbol.'], ['todos los días', 'every day', 'Iba a la escuela todos los días.'], ['normalmente', 'normally, usually', 'Normalmente cenábamos a las seis.'], ['jugar', 'to play', 'Jugaba con mis amigos.'], ['la escuela', 'school', 'Mi escuela era pequeña.'], ['el barrio', 'neighborhood', 'Mi barrio era muy tranquilo.'], ['el perro', 'dog', 'Teníamos un perro grande.'], ['los abuelos', 'grandparents', 'Visitaba a mis abuelos.'], ['el verano', 'summer', 'Todos los veranos íbamos al lago.'], ['tímido / tímida', 'shy', 'Era muy tímido.'], ['soler + infinitivo', 'to usually (do)', 'Solía leer mucho.']],
      grammar: {
        title: 'Imperfecto: the "used to" tense',
        hack: 'Use the imperfecto for habits and descriptions in the past: what things were like and what you used to do. Only THREE irregular verbs: ser (era), ir (iba), ver (veía).',
        tables: [{ head: ['', 'hablar', 'comer / vivir', 'ser', 'ir', 'ver'], rows: [['yo', 'hablaba', 'comía', 'era', 'iba', 'veía'], ['tú', 'hablabas', 'comías', 'eras', 'ibas', 'veías'], ['él / ella / usted', 'hablaba', 'comía', 'era', 'iba', 'veía'], ['nosotros', 'hablábamos', 'comíamos', 'éramos', 'íbamos', 'veíamos'], ['ellos / ustedes', 'hablaban', 'comían', 'eran', 'iban', 'veían']] }],
        examples: [['De niño, vivía en Ohio.', 'As a child, I lived in Ohio.'], ['Todos los veranos íbamos a la playa.', 'Every summer we used to go to the beach.'], ['Mi abuela era muy simpática.', 'My grandmother was very nice.'], ['Era tímido y jugaba mucho con mi perro.', 'I was shy and I played a lot with my dog.']],
        tip: 'Signal words: siempre, todos los días, normalmente, a menudo, de niño, cuando era joven. They usually call for the imperfecto.'
      },
      dialogue: [['Valeria', '¿Dónde vivías cuando eras niño?', 'Where did you live when you were a kid?'], ['Tú', 'Vivía en un barrio pequeño en Ohio. Era muy tranquilo.', 'I lived in a small neighborhood in Ohio. It was very quiet.'], ['Valeria', '¿Y qué hacías después de la escuela?', 'And what did you do after school?'], ['Tú', 'Jugaba al béisbol con mis amigos o veía la tele.', 'I played baseball with my friends or watched TV.'], ['Valeria', '¿Ibas de vacaciones con tu familia?', 'Did you go on vacation with your family?'], ['Tú', 'Sí, todos los veranos íbamos a casa de mis abuelos.', "Yes, every summer we went to my grandparents' house."], ['Valeria', '¡Qué bonito! Yo era muy tímida y leía mucho.', 'How lovely! I was very shy and read a lot.']],
      ex: [
        { t: 'fill', q: 'De niño, yo ___ en Ohio.', a: 'vivía', hint: 'vivir' },
        { t: 'fill', q: 'Todos los veranos nosotros ___ a la playa.', a: 'íbamos', hint: 'ir' },
        { t: 'fill', q: 'Mi abuela ___ muy simpática.', a: 'era', hint: 'ser' },
        { t: 'fill', q: 'Ellos ___ la tele todas las noches.', a: 'veían', hint: 'ver' },
        { t: 'fill', q: '¿Qué ___ tú después de la escuela?', a: 'hacías', hint: 'hacer' },
        { t: 'mc', q: '"Jugaba al béisbol" means…', o: ['I used to play baseball.', 'I played baseball once.', 'I will play baseball.'] },
        { t: 'mc', q: 'Which signal word suggests the imperfecto?', o: ['todos los días', 'ayer', 'el lunes pasado'] },
        { t: 'order', es: 'Cuando era niño vivía en Ohio', en: 'When I was a child, I lived in Ohio' },
        { t: 'listen', es: 'Todos los veranos íbamos a casa de mis abuelos', en: "Every summer we went to my grandparents' house" },
        { t: 'fill', q: 'Mi perro ___ muy grande.', a: 'era', hint: 'ser' }
      ],
      speak: ['Cuando era niño, vivía en Ohio.', 'Todos los veranos íbamos a la playa.', 'Era tímido y leía mucho.'],
      coach: 'The imperfecto has only 3 irregular verbs. That makes it the friendliest past tense — use it freely to describe.'
    },
    {
      title: 'Fui vs. Iba: Choosing the Past', es: 'Fui vs. iba', canDo: 'Choose between pretérito and imperfecto when telling what happened.', trick: 5,
      vocab: [['de repente', 'suddenly', 'De repente, empezó a llover.'], ['mientras', 'while', 'Mientras cocinaba, escuchaba música.'], ['cuando', 'when', 'Cuando llegué, todos dormían.'], ['entonces', 'then, so', 'Llovía, entonces tomé un taxi.'], ['llover', 'to rain', 'Llovía mucho.'], ['hacer frío / calor', 'to be cold / hot (weather)', 'Hacía mucho calor.'], ['el accidente', 'accident', 'Vi un accidente.'], ['el teléfono', 'phone', 'Sonó el teléfono.'], ['sonar', 'to ring, sound', 'El teléfono sonó dos veces.'], ['caminar', 'to walk', 'Caminaba por el parque.'], ['encontrar', 'to find', 'Encontré mis llaves.'], ['mojar', 'to get (something) wet', 'El carro me mojó.']],
      grammar: {
        title: 'The real B1 filter: fui vs. iba',
        hack: 'Think of a movie: the IMPERFECTO is the background scene (what was going on, what it was like); the PRETÉRITO is the action that happens (the event that moves the story forward).',
        tables: [{ head: ['Imperfecto (background)', 'Pretérito (event)'], rows: [['Descriptions: Hacía frío.', 'Completed actions: Salí a las ocho.'], ['Ongoing action: Caminaba…', 'Interrupting action: …cuando sonó el teléfono.'], ['Habits: Iba al gimnasio los lunes.', 'One time: Fui al gimnasio el lunes.'], ['Time and age: Eran las diez. Tenía 20 años.', 'Sequence: Llegué, comí y salí.'], ['mientras…', 'de repente…, ayer…, una vez…']] }],
        examples: [['Caminaba por la calle cuando vi a mi profesor.', 'I was walking down the street when I saw my teacher.'], ['Llovía mucho, entonces tomé un taxi.', 'It was raining a lot, so I took a taxi.'], ['Iba a la playa todos los veranos.', 'I used to go to the beach every summer.'], ['Fui a la playa el verano pasado.', 'I went to the beach last summer.']],
        tip: 'Ask yourself: is it the SCENE or the EVENT? Scene → imperfecto. Event → pretérito.'
      },
      dialogue: [['Tú', '¿Sabes qué me pasó ayer?', 'Do you know what happened to me yesterday?'], ['Amiga', 'No, ¿qué te pasó?', 'No, what happened?'], ['Tú', 'Eran las ocho y llovía muchísimo. Caminaba a la oficina…', 'It was eight and it was pouring. I was walking to the office…'], ['Amiga', '¿Y?', 'And?'], ['Tú', 'De repente, un carro pasó muy rápido y me mojó completamente.', 'Suddenly, a car went by really fast and got me completely wet.'], ['Amiga', '¡No! ¿Y qué hiciste?', 'No! And what did you do?'], ['Tú', 'Volví a casa, me cambié de ropa y llegué tarde a la reunión.', 'I went back home, changed clothes and got to the meeting late.']],
      ex: [
        { t: 'mc', q: 'Ayer ___ al supermercado a las cinco.', o: ['fui', 'iba'], why: 'One completed event at a specific time → pretérito.' },
        { t: 'mc', q: 'De niño, ___ al parque todos los días.', o: ['iba', 'fui'], why: 'Habit → imperfecto.' },
        { t: 'mc', q: '___ las diez de la noche cuando llegué.', o: ['Eran', 'Fueron'], why: 'Clock time in the past is background → imperfecto.' },
        { t: 'mc', q: 'Caminaba por la calle cuando ___ a mi profesor.', o: ['vi', 'veía'], why: 'The interrupting event → pretérito.' },
        { t: 'mc', q: 'Mientras yo ___, mi hermano veía la tele.', o: ['cocinaba', 'cociné'], why: 'Two ongoing background actions with "mientras" → imperfecto.' },
        { t: 'fill', q: 'Ayer ___ mucho, entonces tomé un taxi.', a: 'llovía', hint: 'llover' },
        { t: 'fill', q: 'De repente, ___ el teléfono.', a: 'sonó', hint: 'sonar' },
        { t: 'order', es: 'Llovía mucho cuando salí de casa', en: 'It was raining a lot when I left home' },
        { t: 'listen', es: 'Caminaba a la oficina cuando empezó a llover', en: 'I was walking to the office when it started to rain' },
        { t: 'mc', q: 'La casa ___ grande y tenía un jardín.', o: ['era', 'fue'], why: 'Description → imperfecto.' }
      ],
      speak: ['Eran las ocho y llovía muchísimo.', 'De repente, un carro pasó muy rápido.', 'Caminaba por la calle cuando vi a mi profesor.'],
      coach: 'Train this daily in Practice → Fui vs. Iba. This one skill separates A2 from B1.'
    },
    {
      title: 'Telling a Story with Connectors', es: 'Contar una historia', canDo: 'Tell a short story in the past using connectors.', trick: 5,
      vocab: [['al principio', 'at the beginning', 'Al principio todo iba bien.'], ['más tarde', 'later', 'Más tarde fuimos a cenar.'], ['al final', 'in the end', 'Al final todo salió bien.'], ['por suerte', 'luckily', 'Por suerte, encontré mi cartera.'], ['por desgracia', 'unfortunately', 'Por desgracia, llovió todo el día.'], ['así que', 'so (consequence)', 'Estaba cansado, así que me acosté.'], ['la maleta', 'suitcase', 'Mi maleta no llegó.'], ['el aeropuerto', 'airport', 'Esperé dos horas en el aeropuerto.'], ['la aerolínea', 'airline', 'Llamé a la aerolínea.'], ['preocupado / preocupada', 'worried', 'Estaba muy preocupada.'], ['enojado / enojada', 'angry (Spain: enfadado)', 'Estaba enojado con la aerolínea.'], ['pasar', 'to happen; to spend (time)', '¿Qué pasó?']],
      grammar: {
        title: 'Story structure: scene + events + ending',
        hack: 'A B1 story has three parts: set the scene (imperfecto), tell the events in order (pretérito + connectors), and give the ending or how you felt.',
        tables: [{ head: ['Part', 'Connectors', 'Example'], rows: [['Scene', 'Era…, Hacía…, Estaba…', 'Era domingo y hacía sol.'], ['Start', 'Al principio…, Primero…', 'Al principio todo iba bien.'], ['Events', 'Luego…, Después…, Más tarde…', 'Luego fuimos a la playa.'], ['Twist', 'De repente…, Pero…', 'De repente, empezó a llover.'], ['Ending', 'Al final…, Por suerte…, Así que…', 'Al final, volvimos a casa.']] }],
        examples: [['Al principio todo iba bien.', 'At first everything was going well.'], ['Por desgracia, la aerolínea perdió mi maleta.', 'Unfortunately, the airline lost my suitcase.'], ['Estaba muy preocupado, así que llamé a la aerolínea.', 'I was very worried, so I called the airline.'], ['Por suerte, al final encontraron la maleta.', 'Luckily, in the end they found the suitcase.']],
        tip: 'Feelings at a moment in the story usually take the imperfecto: estaba nervioso, tenía miedo.'
      },
      dialogue: [['Amigo', '¿Qué tal tu viaje a Bogotá?', 'How was your trip to Bogotá?'], ['Tú', 'Al principio todo iba bien. El vuelo fue tranquilo.', 'At first everything was going well. The flight was calm.'], ['Amigo', '¿Y luego?', 'And then?'], ['Tú', 'Luego, en el aeropuerto, esperé una hora, pero mi maleta no llegó.', "Then, at the airport, I waited an hour, but my suitcase didn't arrive."], ['Amigo', '¡Qué mala suerte! ¿Qué hiciste?', 'What bad luck! What did you do?'], ['Tú', 'Estaba muy enojado, así que fui a la oficina de la aerolínea.', 'I was very angry, so I went to the airline office.'], ['Amigo', '¿Y al final?', 'And in the end?'], ['Tú', 'Por suerte, la encontraron y me la llevaron al hotel esa noche.', 'Luckily, they found it and brought it to my hotel that night.']],
      ex: [
        { t: 'mc', q: '"Por suerte" means…', o: ['luckily', 'unfortunately', 'suddenly'] },
        { t: 'mc', q: '"Así que" introduces…', o: ['a consequence (so)', 'a contrast (but)', 'a time (when)'] },
        { t: 'fill', q: '___ todo iba bien.', a: 'al principio', hint: 'At first' },
        { t: 'mc', q: 'Estaba muy cansado, ___ me acosté temprano.', o: ['así que', 'pero', 'mientras'] },
        { t: 'mc', q: 'Perdí el pasaporte, pero…', o: ['por suerte, alguien lo encontró.', 'por desgracia, alguien lo encontró.', 'así que alguien lo encontró.'] },
        { t: 'fill', q: 'Yo ___ muy preocupado.', a: 'estaba', hint: 'estar — a feeling in the scene' },
        { t: 'fill', q: 'Luego ___ a la oficina de la aerolínea.', a: 'fui', hint: 'ir, yo' },
        { t: 'order', es: 'Al final encontraron mi maleta', en: 'In the end they found my suitcase' },
        { t: 'listen', es: 'Por desgracia, la aerolínea perdió mi maleta', en: 'Unfortunately, the airline lost my suitcase' }
      ],
      speak: ['Al principio todo iba bien.', 'Estaba muy enojado, así que fui a la oficina.', 'Por suerte, al final encontraron la maleta.'],
      coach: 'Tell the same story three times this week, a bit faster each time. Repetition builds fluency.'
    },
    // ===================================== PHASE 4 · PLANS, OPINIONS & POLITENESS ===========================
    {
      title: 'Present Perfect: Life Experiences', es: 'El pretérito perfecto: experiencias', canDo: 'Talk about life experiences and what you have (not) done yet.', trick: 4,
      vocab: [['alguna vez', 'ever', '¿Alguna vez has estado en Perú?'], ['ya', 'already', 'Ya he comido.'], ['todavía no', 'not yet', 'Todavía no he terminado.'], ['probar', 'to try, taste', '¿Has probado el mole?'], ['el país', 'country', 'He visitado tres países.'], ['la experiencia', 'experience', 'Fue una buena experiencia.'], ['últimamente', 'lately', 'Últimamente he trabajado mucho.'], ['el extranjero', 'abroad', 'Nunca he vivido en el extranjero.'], ['subir', 'to go up, climb', 'He subido a esa montaña.'], ['romper', 'to break', 'He roto mi teléfono.'], ['el idioma', 'language (masculine!)', 'Hablo dos idiomas.'], ['muchas veces', 'many times', 'Lo he visto muchas veces.']],
      grammar: {
        title: 'He + participle: experiences',
        hack: 'Use haber (he, has, ha, hemos, han) + participle (-ado / -ido) for life experiences (¿Alguna vez has…?) and for things in a period that is not over (hoy, esta semana, ya, todavía no).',
        tables: [
          { head: ['', 'haber', '+ participle'], rows: [['yo', 'he', 'hablado'], ['tú', 'has', 'comido'], ['él / ella / usted', 'ha', 'vivido'], ['nosotros', 'hemos', 'estado'], ['ellos / ustedes', 'han', 'ido']] },
          { head: ['Infinitive', 'Irregular participle'], rows: [['hacer', 'hecho'], ['decir', 'dicho'], ['ver', 'visto'], ['escribir', 'escrito'], ['poner', 'puesto'], ['volver', 'vuelto'], ['abrir', 'abierto'], ['romper', 'roto']] }
        ],
        examples: [['¿Alguna vez has estado en México?', 'Have you ever been to Mexico?'], ['Nunca he probado el mole.', 'I have never tried mole.'], ['Ya he terminado el informe.', 'I have already finished the report.'], ['Todavía no he visto la película.', "I haven't seen the movie yet."]],
        tip: 'In Latin America people often use the pretérito where Spain uses this tense: "Hoy comí tacos" vs. Spain "Hoy he comido tacos". For experiences ("¿Alguna vez has…?"), everyone uses it.'
      },
      dialogue: [['Andrés', '¿Alguna vez has viajado a Sudamérica?', 'Have you ever traveled to South America?'], ['Tú', 'No, nunca he estado allí. ¿Y tú?', "No, I've never been there. And you?"], ['Andrés', 'Sí, he ido a Perú dos veces. He subido a Machu Picchu.', "Yes, I've been to Peru twice. I've climbed Machu Picchu."], ['Tú', '¡Qué envidia! ¿Has probado el ceviche?', "I'm so jealous! Have you tried ceviche?"], ['Andrés', 'Muchas veces. ¿Tú todavía no lo has probado?', "Many times. You haven't tried it yet?"], ['Tú', 'Todavía no, pero quiero probarlo pronto.', 'Not yet, but I want to try it soon.']],
      ex: [
        { t: 'fill', q: '¿Alguna vez ___ estado en Cuba?', a: 'has', hint: 'haber, tú' },
        { t: 'fill', q: 'Nosotros ya ___ comido.', a: 'hemos', hint: 'haber' },
        { t: 'fill', q: 'Nunca he ___ esa película.', a: 'visto', hint: 'ver' },
        { t: 'fill', q: '¿Qué has ___ hoy?', a: 'hecho', hint: 'hacer' },
        { t: 'mc', q: '"Todavía no he terminado." means…', o: ["I haven't finished yet.", 'I already finished.', 'I never finish.'] },
        { t: 'mc', q: 'The participle of "escribir" is…', o: ['escrito', 'escribido', 'escribo'] },
        { t: 'order', es: 'Nunca he probado el mole', en: 'I have never tried mole' },
        { t: 'listen', es: '¿Alguna vez has viajado a Sudamérica?', en: 'Have you ever traveled to South America?' },
        { t: 'fill', q: 'Ellos ___ vuelto de su viaje.', a: 'han', hint: 'haber' }
      ],
      speak: ['¿Alguna vez has estado en México?', 'Nunca he probado el mole.', 'Todavía no he visto la película.'],
      coach: 'A great conversation starter in exchanges: "¿Alguna vez has…?" It keeps people talking.'
    },
    {
      title: 'Future Tense & Predictions', es: 'El futuro y las predicciones', canDo: 'Make predictions and talk about future plans and conditions.', trick: 4,
      vocab: [['el futuro', 'future', 'Pienso mucho en el futuro.'], ['el próximo año', 'next year', 'El próximo año viajaré más.'], ['dentro de cinco años', 'in five years', '¿Dónde estarás dentro de cinco años?'], ['seguramente', 'surely, probably', 'Seguramente lloverá.'], ['tal vez', 'maybe', 'Tal vez me mude.'], ['mudarse', 'to move (house)', 'Me mudaré en marzo.'], ['ahorrar', 'to save (money)', 'Quiero ahorrar dinero.'], ['la carrera', 'career; degree', 'Mi carrera es importante.'], ['el ascenso', 'promotion', 'Espero conseguir un ascenso.'], ['conseguir', 'to get, achieve', 'Conseguiré mi meta.'], ['el sueño', 'dream; sleepiness', 'Mi sueño es vivir en México.'], ['si', 'if', 'Si puedo, iré.']],
      grammar: {
        title: 'Futuro simple: infinitive + endings',
        hack: 'Add -é, -ás, -á, -emos, -án to the WHOLE infinitive. Irregular verbs change the stem but keep the same endings.',
        tables: [
          { head: ['', 'hablar', 'tener → tendr-', 'hacer → har-'], rows: [['yo', 'hablaré', 'tendré', 'haré'], ['tú', 'hablarás', 'tendrás', 'harás'], ['él / ella / usted', 'hablará', 'tendrá', 'hará'], ['nosotros', 'hablaremos', 'tendremos', 'haremos'], ['ellos / ustedes', 'hablarán', 'tendrán', 'harán']] },
          { head: ['Infinitive', 'Future stem'], rows: [['poder', 'podr-'], ['salir', 'saldr-'], ['venir', 'vendr-'], ['decir', 'dir-'], ['querer', 'querr-'], ['saber', 'sabr-'], ['poner', 'pondr-']] }
        ],
        examples: [['El próximo año viviré en otro país.', 'Next year I will live in another country.'], ['Si ahorro dinero, viajaré a Chile.', 'If I save money, I will travel to Chile.'], ['Seguramente lloverá mañana.', 'It will probably rain tomorrow.'], ['¿Qué harás dentro de cinco años?', 'What will you do in five years?']],
        tip: 'Conditions: Si + present, future. "Si tengo tiempo, te llamaré." Never use the future right after "si".'
      },
      dialogue: [['Laura', '¿Cómo te imaginas dentro de cinco años?', 'How do you picture yourself in five years?'], ['Tú', 'Creo que hablaré español con fluidez y trabajaré con clientes en Latinoamérica.', "I think I'll speak Spanish fluently and work with clients in Latin America."], ['Laura', '¿Te mudarás a otro país?', 'Will you move to another country?'], ['Tú', 'Tal vez. Si consigo un ascenso, me mudaré a Ciudad de México.', "Maybe. If I get a promotion, I'll move to Mexico City."], ['Laura', '¡Qué emocionante! ¿Y qué harás primero?', 'How exciting! And what will you do first?'], ['Tú', 'Primero ahorraré dinero. Después, ¡buscaré un apartamento con terraza!', "First I'll save money. Then I'll look for an apartment with a terrace!"]],
      ex: [
        { t: 'fill', q: 'El próximo año yo ___ en Chile.', a: 'viviré', hint: 'vivir' },
        { t: 'fill', q: 'Nosotros ___ más tiempo libre.', a: 'tendremos', hint: 'tener' },
        { t: 'fill', q: '¿Qué ___ tú mañana?', a: 'harás', hint: 'hacer' },
        { t: 'fill', q: 'Ellos no ___ venir a la fiesta.', a: 'podrán', hint: 'poder' },
        { t: 'mc', q: 'Si ___ tiempo, te llamaré.', o: ['tengo', 'tendré'], why: 'After "si" use the present, not the future.' },
        { t: 'mc', q: '"Seguramente lloverá" means…', o: ['It will probably rain.', 'It is surely raining.', 'It rained a lot.'] },
        { t: 'order', es: 'Si ahorro dinero viajaré a Chile', en: 'If I save money, I will travel to Chile' },
        { t: 'listen', es: '¿Qué harás dentro de cinco años?', en: 'What will you do in five years?' },
        { t: 'fill', q: 'Te lo ___ mañana.', a: 'diré', hint: 'decir, yo' }
      ],
      speak: ['Dentro de cinco años hablaré español con fluidez.', 'Si consigo un ascenso, me mudaré.', 'Primero ahorraré dinero.'],
      coach: 'Use "ir a + infinitive" for decided plans and the futuro simple for predictions and promises.'
    },
    {
      title: 'Conditional: Politeness & Advice', es: 'El condicional: cortesía y consejos', canDo: 'Make polite requests and give advice.', trick: 4,
      vocab: [['me gustaría', 'I would like', 'Me gustaría hablar con usted.'], ['podría', 'could (I / you, formal)', '¿Podría ayudarme?'], ['debería', 'should', 'Deberías descansar.'], ['el consejo', 'piece of advice', 'Te doy un consejo.'], ['el jefe / la jefa', 'boss', 'Mi jefa es muy justa.'], ['el correo', 'email; mail', 'Te mando un correo.'], ['la entrevista', 'interview', 'Tengo una entrevista mañana.'], ['ayudar', 'to help', '¿Me puedes ayudar?'], ['cambiar', 'to change', 'Quiero cambiar mi horario.'], ['el horario', 'schedule', 'Mi horario es flexible.'], ['yo que tú', 'if I were you', 'Yo que tú, lo haría.'], ['mejor', 'better', 'Es mejor preguntar.']],
      grammar: {
        title: 'Condicional: softer and smarter',
        hack: 'Same stem as the future + -ía endings. Use it to sound polite (¿Podría…?), express wishes (Me gustaría…) and give advice (Deberías… / Yo que tú…).',
        tables: [{ head: ['', 'hablar', 'poder → podr-', 'tener → tendr-'], rows: [['yo', 'hablaría', 'podría', 'tendría'], ['tú', 'hablarías', 'podrías', 'tendrías'], ['él / ella / usted', 'hablaría', 'podría', 'tendría'], ['nosotros', 'hablaríamos', 'podríamos', 'tendríamos'], ['ellos / ustedes', 'hablarían', 'podrían', 'tendrían']] }],
        examples: [['¿Podría ayudarme, por favor?', 'Could you help me, please?'], ['Me gustaría cambiar mi horario.', 'I would like to change my schedule.'], ['Deberías dormir más.', 'You should sleep more.'], ['Yo que tú, hablaría con el jefe.', 'If I were you, I would talk to the boss.']],
        tip: '"Quisiera" (from querer) is an extra-polite alternative to "me gustaría": Quisiera hacer una pregunta.'
      },
      dialogue: [['Tú', 'Disculpa, ¿podrías darme un consejo?', 'Excuse me, could you give me some advice?'], ['Colega', 'Claro, ¿qué pasa?', "Sure, what's up?"], ['Tú', 'Tengo una entrevista mañana y estoy nervioso.', "I have an interview tomorrow and I'm nervous."], ['Colega', 'Yo que tú, prepararía tres ejemplos de tu experiencia.', 'If I were you, I would prepare three examples from your experience.'], ['Tú', 'Buena idea. ¿Debería hablar de mi español?', 'Good idea. Should I talk about my Spanish?'], ['Colega', 'Sí, deberías mencionarlo. Les encantaría saberlo.', "Yes, you should mention it. They'd love to know."], ['Tú', 'Muchas gracias. Me gustaría invitarte a un café después.', "Thank you so much. I'd like to buy you a coffee afterwards."]],
      ex: [
        { t: 'fill', q: '¿___ ayudarme, por favor?', a: 'podría', hint: 'poder, usted' },
        { t: 'fill', q: 'Me ___ cambiar mi horario.', a: 'gustaría', hint: 'gustar' },
        { t: 'fill', q: 'Tú ___ dormir más.', a: 'deberías', hint: 'deber' },
        { t: 'mc', q: 'The most polite request:', o: ['¿Podría traerme un vaso de agua?', 'Tráeme agua.', 'Quiero agua.'] },
        { t: 'mc', q: '"Yo que tú…" means…', o: ['If I were you…', 'You and I…', 'I want you…'] },
        { t: 'fill', q: 'Yo que tú, ___ con el jefe.', a: 'hablaría', hint: 'hablar' },
        { t: 'order', es: 'Me gustaría cambiar mi horario', en: 'I would like to change my schedule' },
        { t: 'listen', es: '¿Podrías darme un consejo?', en: 'Could you give me some advice?' },
        { t: 'fill', q: 'Con más dinero, nosotros ___ una casa más grande.', a: 'tendríamos', hint: 'tener' }
      ],
      speak: ['¿Podría ayudarme, por favor?', 'Me gustaría cambiar mi horario.', 'Yo que tú, hablaría con el jefe.'],
      coach: 'At B1, politeness matters more than perfection. "¿Podría…?" opens doors.'
    },
    {
      title: 'Comparing & Giving Opinions', es: 'Comparar y opinar', canDo: 'Compare things and give your opinion with reasons.', trick: 4,
      vocab: [['más … que', 'more … than', 'Es más grande que mi casa.'], ['menos … que', 'less … than', 'Es menos caro que el avión.'], ['tan … como', 'as … as', 'Es tan bonito como el otro.'], ['peor', 'worse', 'El tráfico es peor hoy.'], ['mayor', 'older; bigger', 'Mi hermano es mayor que yo.'], ['menor', 'younger; smaller', 'Mi hermana es menor.'], ['creo que', 'I think that', 'Creo que tienes razón.'], ['en mi opinión', 'in my opinion', 'En mi opinión, es mejor así.'], ['estar de acuerdo', 'to agree', 'Estoy de acuerdo contigo.'], ['la ventaja', 'advantage', 'Tiene muchas ventajas.'], ['la desventaja', 'disadvantage', 'La desventaja es el precio.'], ['tranquilo / tranquila', 'calm, quiet', 'Es un barrio tranquilo.']],
      grammar: {
        title: 'Compare + give opinions with reasons',
        hack: 'Comparisons: más / menos + adjective + que; tan + adjective + como. Opinions: Creo que…, Pienso que…, En mi opinión…, (No) estoy de acuerdo. Then add a reason with "porque".',
        tables: [{ head: ['Pattern', 'Example'], rows: [['más + adj. + que', 'Austin es más grande que Boulder.'], ['menos + adj. + que', 'El tren es menos caro que el avión.'], ['tan + adj. + como', 'Mi español es tan bueno como mi francés.'], ['mejor / peor que', 'Este restaurante es mejor que el otro.'], ['mayor / menor que', 'Mi hermano es mayor que yo.']] }],
        examples: [['Creo que vivir en una ciudad pequeña es más tranquilo.', 'I think living in a small city is calmer.'], ['En mi opinión, el transporte público es mejor.', 'In my opinion, public transportation is better.'], ['No estoy de acuerdo, porque es más lento.', "I don't agree, because it's slower."], ['Una ventaja es que es más barato.', 'An advantage is that it is cheaper.']],
        tip: 'Never say "más bueno" or "más malo" for quality: use mejor and peor.'
      },
      dialogue: [['Isabel', '¿Qué prefieres, vivir en el centro o en las afueras?', 'Which do you prefer, living downtown or in the suburbs?'], ['Tú', 'Creo que es mejor vivir en las afueras. Es más tranquilo y menos caro.', "I think it's better to live in the suburbs. It's quieter and less expensive."], ['Isabel', 'No estoy de acuerdo. En el centro todo está más cerca.', "I don't agree. Downtown everything is closer."], ['Tú', 'Es verdad, pero el centro es tan ruidoso como Nueva York.', "That's true, but downtown is as noisy as New York."], ['Isabel', '¡Exageras! En mi opinión, la ventaja del centro es la vida social.', "You're exaggerating! In my opinion, the advantage of downtown is the social life."], ['Tú', 'Bueno, para mí la desventaja es el tráfico. Es peor que en las afueras.', "Well, for me the disadvantage is the traffic. It's worse than in the suburbs."]],
      ex: [
        { t: 'mc', q: 'El avión es ___ rápido ___ el tren.', o: ['más … que', 'tan … que', 'más … como'] },
        { t: 'mc', q: 'Fix the mistake: "Este café es más bueno que el otro."', o: ['Este café es mejor que el otro.', 'Este café es más mejor que el otro.', 'Este café es bueno que el otro.'] },
        { t: 'fill', q: 'Mi hermana es ___ que yo.', a: 'mayor', hint: 'older' },
        { t: 'fill', q: 'Madrid es tan bonita ___ Barcelona.', a: 'como', hint: 'as … as' },
        { t: 'mc', q: '"No estoy de acuerdo" means…', o: ["I don't agree.", "I'm not ready.", "I don't remember."] },
        { t: 'fill', q: '___ que el transporte público es mejor.', a: 'creo', hint: 'I think' },
        { t: 'order', es: 'En mi opinión el tren es mejor', en: 'In my opinion, the train is better' },
        { t: 'listen', es: 'Una ventaja es que es más barato', en: 'An advantage is that it is cheaper' },
        { t: 'mc', q: 'The best B1 answer:', o: ['Creo que es mejor porque es más barato.', 'Creo que es mejor.', 'Es mejor, sí.'], why: 'B1 = opinion + reason.' }
      ],
      speak: ['Creo que es mejor vivir en las afueras.', 'No estoy de acuerdo, porque es más lento.', 'En mi opinión, la ventaja es el precio.'],
      coach: 'Examiners love "opinion + reason + example". Practise it on any topic: coffee vs. tea, city vs. countryside.'
    },
    {
      title: 'Por vs. Para', es: 'Por y para', canDo: 'Use por and para correctly in everyday situations.', trick: 5,
      vocab: [['el regalo', 'gift', 'Este regalo es para ti.'], ['el boleto', 'ticket (Spain: billete)', 'Compré el boleto por internet.'], ['el autobús', 'bus', 'El autobús pasa por el centro.'], ['la semana que viene', 'next week', 'Lo necesito para la semana que viene.'], ['gracias por', 'thanks for', 'Gracias por tu ayuda.'], ['por la mañana', 'in the morning', 'Estudio por la mañana.'], ['para mí', 'for me; in my opinion', 'Para mí, es fácil.'], ['el parque', 'park', 'Caminamos por el parque.'], ['salir para', 'to leave for', 'Mañana salgo para Lima.'], ['el cliente / la clienta', 'client', 'Viajo para visitar a un cliente.'], ['la fecha límite', 'deadline', 'La fecha límite es el viernes.'], ['por fin', 'finally, at last', '¡Por fin es viernes!']],
      grammar: {
        title: 'Por vs. para — the simple version',
        hack: 'PARA points to a goal or destination (purpose, recipient, deadline, destination). POR is the route or the reason (cause, through, exchange, duration, means).',
        tables: [{ head: ['PARA → goal', 'POR → route / reason'], rows: [['Purpose: Estudio para hablar con clientes.', 'Cause: Gracias por tu ayuda.'], ['Recipient: El regalo es para ti.', 'Through: Caminamos por el parque.'], ['Destination: Salgo para Lima.', 'Exchange: Pagué 50 dólares por el boleto.'], ['Deadline: Es para el viernes.', 'Duration: Estudié por dos horas.'], ['Opinion: Para mí, es fácil.', 'Means: Hablamos por teléfono.']] }],
        examples: [['Este regalo es para mi mamá.', 'This gift is for my mom.'], ['Gracias por todo.', 'Thanks for everything.'], ['Necesito el informe para el lunes.', 'I need the report by Monday.'], ['Hablamos por teléfono por la mañana.', 'We talked on the phone in the morning.']],
        tip: 'Fixed expressions with por: por favor, por ejemplo, por fin, por eso, por supuesto.'
      },
      dialogue: [['Tú', 'Hola, quisiera un boleto para Guadalajara, por favor.', "Hi, I'd like a ticket to Guadalajara, please."], ['Empleado', '¿Para cuándo?', 'For when?'], ['Tú', 'Para el viernes por la mañana.', 'For Friday morning.'], ['Empleado', 'Hay un autobús a las ocho. Pasa por León.', 'There is a bus at eight. It goes through León.'], ['Tú', 'Perfecto. ¿Cuánto es?', 'Perfect. How much is it?'], ['Empleado', 'Son seiscientos pesos. ¿Paga con tarjeta?', 'Six hundred pesos. Paying by card?'], ['Tú', 'Sí. Es un viaje para visitar a un cliente. ¡Gracias por su ayuda!', "Yes. It's a trip to visit a client. Thanks for your help!"]],
      ex: [
        { t: 'mc', q: 'Este regalo es ___ ti.', o: ['para', 'por'], why: 'Recipient → para.' },
        { t: 'mc', q: 'Gracias ___ tu ayuda.', o: ['por', 'para'], why: 'Cause / reason → por.' },
        { t: 'mc', q: 'Caminamos ___ el parque.', o: ['por', 'para'], why: 'Through → por.' },
        { t: 'mc', q: 'Necesito el informe ___ el lunes.', o: ['para', 'por'], why: 'Deadline → para.' },
        { t: 'mc', q: 'Estudio español ___ hablar con mis clientes.', o: ['para', 'por'], why: 'Purpose → para.' },
        { t: 'fill', q: 'Pagué 30 dólares ___ el boleto.', a: 'por', hint: 'exchange', why: 'Exchange → por.' },
        { t: 'fill', q: 'Hablamos ___ teléfono ayer.', a: 'por', hint: 'means', why: 'Means of communication → por.' },
        { t: 'order', es: 'Quisiera un boleto para Guadalajara', en: 'I would like a ticket to Guadalajara' },
        { t: 'listen', es: 'Necesito el informe para el viernes', en: 'I need the report by Friday' }
      ],
      speak: ['Quisiera un boleto para el viernes.', 'Gracias por su ayuda.', 'Estudio español para hablar con mis clientes.'],
      coach: 'Don\'t freeze over por/para in conversation. People will understand you — fix it later in review.'
    },
    // ===================================== PHASE 5 · REAL-WORLD B1 ==========================================
    {
      title: 'Subjunctive: Recognize It, Don\'t Obsess', es: 'El subjuntivo: reconocerlo', canDo: 'Recognize the subjunctive in wishes and advice, and use a few fixed phrases.', trick: 5,
      vocab: [['ojalá', 'I hope / hopefully', 'Ojalá haga buen tiempo.'], ['espero que', 'I hope that', 'Espero que estés bien.'], ['quiero que', 'I want (someone) to', 'Quiero que vengas.'], ['es importante que', 'it is important that', 'Es importante que descanses.'], ['para que', 'so that', 'Te lo explico para que entiendas.'], ['recomendar', 'to recommend', 'Te recomiendo este libro.'], ['el éxito', 'success (false friend: not "exit")', '¡Mucho éxito mañana!'], ['la suerte', 'luck', '¡Buena suerte!'], ['¡Que te vaya bien!', 'Hope it goes well for you!', '¡Que te vaya bien en el viaje!'], ['¡Que tengas un buen día!', 'Have a nice day!', 'Gracias, ¡que tengas un buen día!'], ['mejorar', 'to improve', 'Mi español va a mejorar.'], ['el público', 'audience; public', 'El público fue muy amable.']],
      grammar: {
        title: 'Subjunctive: understand it, don\'t obsess',
        hack: 'For B1 you need to RECOGNIZE the subjunctive and use a few chunks. It appears after wishes, recommendations and emotions about SOMEONE ELSE: Quiero que tú vengas. Form: take the yo form, drop the -o, swap the vowel (-ar → e, -er/-ir → a).',
        tables: [{ head: ['Infinitive', 'yo (present)', 'Subjunctive', 'Example'], rows: [['hablar', 'hablo', 'hable', 'Quiero que hable más.'], ['comer', 'como', 'coma', 'Espero que coma bien.'], ['tener', 'tengo', 'tenga', 'Ojalá tenga tiempo.'], ['hacer', 'hago', 'haga', 'Es importante que lo haga.'], ['venir', 'vengo', 'venga', 'Quiero que venga.'], ['ser', '—', 'sea', 'Ojalá sea fácil.'], ['ir', '—', 'vaya', '¡Que te vaya bien!'], ['estar', '—', 'esté', 'Espero que estés bien.']] }],
        examples: [['Ojalá tengas un buen viaje.', 'I hope you have a good trip.'], ['Quiero que vengas a la fiesta.', 'I want you to come to the party.'], ['Es importante que practiques todos los días.', 'It is important that you practice every day.'], ['Cuando llegues, llámame.', 'When you arrive, call me.']],
        tip: 'Same subject → infinitive: Quiero ir. Different subject → que + subjunctive: Quiero que tú vayas.'
      },
      dialogue: [['Profesora', 'Mañana es tu presentación, ¿verdad?', "Tomorrow is your presentation, right?"], ['Tú', 'Sí, y estoy nervioso. Espero que todo salga bien.', "Yes, and I'm nervous. I hope everything goes well."], ['Profesora', 'Es importante que hables despacio y con calma.', 'It is important that you speak slowly and calmly.'], ['Tú', '¿Me recomienda practicar con alguien?', 'Do you recommend that I practice with someone?'], ['Profesora', 'Sí, te recomiendo que practiques con un compañero esta noche.', 'Yes, I recommend that you practice with a classmate tonight.'], ['Tú', 'Gracias. Ojalá el público no haga preguntas difíciles.', "Thanks. Hopefully the audience won't ask difficult questions."], ['Profesora', '¡Lo harás muy bien! Que te vaya bien.', "You'll do great! Good luck."]],
      ex: [
        { t: 'mc', q: 'Which sentence uses the subjunctive?', o: ['Quiero que vengas.', 'Quiero venir.', 'Vengo mañana.'] },
        { t: 'mc', q: '"Ojalá tengas suerte." means…', o: ['I hope you are lucky.', 'You are lucky.', 'You were lucky.'] },
        { t: 'mc', q: '"I want to go" =', o: ['Quiero ir.', 'Quiero que vaya.'], why: 'Same subject → infinitive.' },
        { t: 'mc', q: 'Quiero que tú ___ conmigo.', o: ['vengas', 'vienes', 'venir'], why: 'Different subject → que + subjunctive.' },
        { t: 'fill', q: 'Es importante que ___ todos los días.', a: 'practiques', hint: 'practicar, tú', why: 'Spelling: c → qu before e.' },
        { t: 'mc', q: '"¡Que te vaya bien!" is used to…', o: ['wish someone well', 'ask for directions', 'complain'] },
        { t: 'mc', q: '"Cuando llegues, llámame." refers to…', o: ['the future', 'the past', 'a habit'], why: 'cuando + subjunctive = a future moment.' },
        { t: 'order', es: 'Espero que todo salga bien', en: 'I hope everything goes well' },
        { t: 'listen', es: 'Ojalá tengas un buen viaje', en: 'I hope you have a good trip' }
      ],
      speak: ['Espero que todo salga bien.', 'Ojalá tengas un buen viaje.', '¡Que te vaya bien!'],
      coach: 'Learn five chunks and stop there: Ojalá…, Espero que…, Quiero que…, Es importante que…, ¡Que te vaya bien! That covers most B1 situations.'
    },
    {
      title: 'Health: At the Doctor', es: 'La salud: en el médico', canDo: 'Explain symptoms and understand a doctor\'s advice.', trick: 4,
      vocab: [['la cabeza', 'head', 'Me duele la cabeza.'], ['el estómago', 'stomach', 'Me duele el estómago.'], ['la garganta', 'throat', 'Tengo la garganta roja.'], ['la espalda', 'back', 'Me duele la espalda.'], ['la fiebre', 'fever', 'Tengo fiebre.'], ['la tos', 'cough', 'Tengo mucha tos.'], ['el dolor', 'pain', 'Tengo un dolor fuerte.'], ['la receta', 'prescription; recipe', 'Necesito una receta.'], ['la pastilla', 'pill', 'Tome una pastilla.'], ['enfermo / enferma', 'sick', 'Estoy enfermo.'], ['la cita', 'appointment', 'Tengo una cita a las diez.'], ['desde hace', 'for (a time, still ongoing)', 'Estudio español desde hace un año.']],
      grammar: {
        title: 'Doler + symptoms + "desde hace"',
        hack: 'Doler works like gustar: Me duele la cabeza (one thing), Me duelen los pies (plural). Say how long with "desde hace": Tengo fiebre desde hace dos días.',
        tables: [{ head: ['Spanish', 'English'], rows: [['Me duele la cabeza.', 'I have a headache.'], ['Me duelen los pies.', 'My feet hurt.'], ['Tengo fiebre / tos.', 'I have a fever / a cough.'], ['Estoy resfriado / resfriada.', 'I have a cold.'], ['Me siento mal / mareado.', 'I feel sick / dizzy.'], ['Desde hace tres días.', 'For three days (and still).']] }],
        examples: [['Me duele mucho la garganta.', 'My throat hurts a lot.'], ['Tengo tos desde hace una semana.', "I've had a cough for a week."], ['Tome una pastilla cada ocho horas.', 'Take a pill every eight hours.'], ['Necesito una cita con el médico.', 'I need an appointment with the doctor.']],
        tip: 'False friend: "constipado" means having a cold (in Spain), not constipated. "Estreñido" = constipated.'
      },
      dialogue: [['Doctora', 'Buenos días. ¿Qué le pasa?', "Good morning. What's wrong?"], ['Tú', 'Me siento mal. Me duele la garganta y tengo tos.', 'I feel sick. My throat hurts and I have a cough.'], ['Doctora', '¿Desde cuándo?', 'Since when?'], ['Tú', 'Desde hace tres días. Anoche también tuve fiebre.', 'For three days. Last night I had a fever too.'], ['Doctora', 'A ver… Tiene la garganta muy roja. Le voy a dar una receta.', "Let's see… Your throat is very red. I'm going to give you a prescription."], ['Tú', '¿Cada cuánto tengo que tomar las pastillas?', 'How often do I have to take the pills?'], ['Doctora', 'Tome una cada ocho horas y descanse. Beba mucha agua.', 'Take one every eight hours and rest. Drink lots of water.']],
      ex: [
        { t: 'fill', q: 'Me ___ la cabeza.', a: 'duele', hint: 'doler' },
        { t: 'fill', q: 'Me ___ los pies.', a: 'duelen', hint: 'doler' },
        { t: 'mc', q: '"Tengo tos desde hace una semana." means…', o: ["I've had a cough for a week.", 'I had a cough a week ago.', 'I will have a cough for a week.'] },
        { t: 'mc', q: 'The doctor says "Tome una pastilla cada ocho horas". You should…', o: ['take one pill every eight hours', 'take eight pills', 'take one pill at eight'] },
        { t: 'mc', q: '"Estoy constipado" (Spain) means…', o: ['I have a cold.', 'I am constipated.', 'I am tired.'] },
        { t: 'fill', q: 'Tengo ___: 39 grados.', a: 'fiebre', hint: 'fever' },
        { t: 'order', es: 'Me duele mucho la garganta', en: 'My throat hurts a lot' },
        { t: 'listen', es: 'Necesito una cita con el médico', en: 'I need an appointment with the doctor' },
        { t: 'fill', q: 'Ayer yo ___ fiebre.', a: 'tuve', hint: 'tener' }
      ],
      speak: ['Me duele la garganta y tengo tos.', 'Tengo fiebre desde hace dos días.', '¿Cada cuánto tengo que tomar las pastillas?'],
      coach: 'Before a real appointment, write your symptoms in Spanish. Preparing = confidence.'
    },
    {
      title: 'Work & Professional Life', es: 'El trabajo', canDo: 'Describe your job and responsibilities, and write a short work email.', trick: 4,
      vocab: [['la empresa', 'company', 'Trabajo en una empresa grande.'], ['el equipo', 'team; equipment', 'Mi equipo es excelente.'], ['el proyecto', 'project', 'El proyecto termina en mayo.'], ['el informe', 'report', 'Tengo que escribir un informe.'], ['encargarse de', 'to be in charge of', 'Me encargo de las ventas.'], ['la responsabilidad', 'responsibility', 'Tengo muchas responsabilidades.'], ['el sueldo', 'salary', 'El sueldo es bueno.'], ['el compañero de trabajo', 'coworker', 'Mis compañeros de trabajo son simpáticos.'], ['además', 'besides, also', 'Además, hablo con clientes.'], ['sin embargo', 'however', 'Sin embargo, es estresante.'], ['aunque', 'although', 'Aunque es difícil, me gusta.'], ['por eso', "that's why", 'Por eso estudio español.']],
      grammar: {
        title: 'Connectors that sound B1',
        hack: 'Join ideas to sound more advanced: además (also), sin embargo (however), aunque (although), por eso (that\'s why), mientras que (whereas). Describe responsibilities with "me encargo de + noun / infinitive".',
        tables: [{ head: ['Connector', 'Use', 'Example'], rows: [['además', 'add', 'Además, hablo con clientes.'], ['sin embargo', 'contrast', 'Me gusta; sin embargo, es estresante.'], ['aunque', 'concession', 'Aunque es difícil, me encanta.'], ['por eso', 'consequence', 'Viajo mucho; por eso aprendo español.'], ['mientras que', 'compare', 'Yo vendo, mientras que mi equipo diseña.']] }],
        examples: [['Trabajo en una empresa de software.', 'I work at a software company.'], ['Me encargo de las ventas en Latinoamérica.', "I'm in charge of sales in Latin America."], ['Aunque es estresante, me encanta mi trabajo.', 'Although it is stressful, I love my job.'], ['Viajo mucho; por eso necesito español.', "I travel a lot; that's why I need Spanish."]],
        tip: 'Email openings: "Estimado/a…:" (formal), "Hola, …:" (friendly). Closings: "Saludos cordiales", "Un saludo".'
      },
      dialogue: [['Entrevistadora', 'Cuénteme sobre su trabajo actual.', 'Tell me about your current job.'], ['Tú', 'Trabajo en una empresa de tecnología. Me encargo de un equipo de cinco personas.', "I work at a tech company. I'm in charge of a team of five people."], ['Entrevistadora', '¿Cuáles son sus responsabilidades?', 'What are your responsibilities?'], ['Tú', 'Organizo los proyectos y escribo informes. Además, hablo con clientes.', 'I organize projects and write reports. I also talk to clients.'], ['Entrevistadora', '¿Y por qué le interesa este puesto?', 'And why are you interested in this position?'], ['Tú', 'Aunque me gusta mi trabajo, quiero trabajar con Latinoamérica. Por eso estudio español.', "Although I like my job, I want to work with Latin America. That's why I study Spanish."]],
      ex: [
        { t: 'mc', q: '"Sin embargo" means…', o: ['however', 'without', 'therefore'] },
        { t: 'mc', q: '"Mi trabajo actual" means…', o: ['my current job', 'my actual job', 'my real job'], why: 'False friend: actual = current. "Actually" = en realidad.' },
        { t: 'fill', q: 'Me ___ de las ventas.', a: 'encargo', hint: 'encargarse, yo' },
        { t: 'mc', q: 'Aunque es difícil, …', o: ['me encanta.', 'por eso me encanta.', 'sin embargo.'] },
        { t: 'mc', q: 'Viajo mucho; ___ necesito español.', o: ['por eso', 'aunque', 'sin embargo'] },
        { t: 'fill', q: 'Hablo inglés y, ___, hablo un poco de español.', a: 'además', hint: 'besides' },
        { t: 'order', es: 'Trabajo en una empresa de tecnología', en: 'I work at a technology company' },
        { t: 'listen', es: 'Aunque es estresante, me encanta mi trabajo', en: 'Although it is stressful, I love my job' },
        { t: 'mc', q: 'A formal email opening:', o: ['Estimada señora López:', '¡Hola, López!', 'Oye, López'] }
      ],
      speak: ['Trabajo en una empresa de tecnología.', 'Me encargo de un equipo de cinco personas.', 'Viajo mucho; por eso estudio español.'],
      coach: 'Prepare a one-minute "elevator pitch" about your job in Spanish. You will use it in every exchange.'
    },
    {
      title: 'Travel Problems: Solving Them', es: 'Viajes y problemas', canDo: 'Solve problems while traveling: lost items, complaints and changes.', trick: 4,
      vocab: [['la reserva', 'reservation', 'Tengo una reserva para hoy.'], ['la habitación', 'room (hotel)', 'La habitación es pequeña.'], ['la llave', 'key', 'Se me olvidó la llave.'], ['el aire acondicionado', 'air conditioning', 'El aire acondicionado no funciona.'], ['funcionar', 'to work (machines)', 'El wifi no funciona.'], ['quejarse', 'to complain', 'Quiero quejarme.'], ['olvidar', 'to forget', 'No olvides tu pasaporte.'], ['la recepción', 'front desk', 'Pregunte en la recepción.'], ['el reembolso', 'refund', 'Quisiera un reembolso.'], ['cancelar', 'to cancel', 'Cancelaron el vuelo.'], ['el retraso', 'delay', 'Hay un retraso de una hora.'], ['la billetera', 'wallet (also: la cartera)', 'Perdí mi billetera.']],
      grammar: {
        title: 'Accidental "se": it got lost on me',
        hack: 'To say something happened by accident, Spanish uses se + me / te / le + verb: Se me olvidó la llave (I forgot the key — literally "the key forgot itself on me"). The verb agrees with the thing.',
        tables: [{ head: ['Spanish', 'Literally', 'English'], rows: [['Se me olvidó la llave.', 'The key got forgotten on me.', 'I forgot the key.'], ['Se me perdieron las llaves.', 'The keys got lost on me.', 'I lost my keys.'], ['Se me rompió el teléfono.', 'The phone broke on me.', 'My phone broke.'], ['Se nos acabó el dinero.', 'The money ran out on us.', 'We ran out of money.']] }],
        examples: [['Se me olvidó la llave en la habitación.', 'I left the key in the room.'], ['El aire acondicionado no funciona.', "The air conditioning doesn't work."], ['Quisiera cambiar de habitación, por favor.', "I'd like to change rooms, please."], ['El vuelo tiene un retraso de dos horas.', 'The flight has a two-hour delay.']],
        tip: 'To complain politely: "Disculpe, tengo un problema con…" + conditional: "¿Sería posible…?"'
      },
      dialogue: [['Tú', 'Buenas noches. Disculpe, tengo un problema con mi habitación.', 'Good evening. Excuse me, I have a problem with my room.'], ['Recepcionista', '¿Qué pasó?', 'What happened?'], ['Tú', 'El aire acondicionado no funciona y hace mucho calor.', "The air conditioning doesn't work and it's very hot."], ['Recepcionista', 'Lo siento mucho. ¿Quiere cambiar de habitación?', "I'm very sorry. Would you like to change rooms?"], ['Tú', 'Sí, por favor. Ah, y se me olvidó la llave adentro.', 'Yes, please. Oh, and I left the key inside.'], ['Recepcionista', 'No se preocupe. Aquí tiene una llave nueva para la habitación 305.', "Don't worry. Here's a new key for room 305."], ['Tú', 'Muchísimas gracias, muy amable.', 'Thank you so much, very kind.']],
      ex: [
        { t: 'mc', q: '"Se me olvidó el pasaporte." means…', o: ['I forgot my passport.', 'I lost my passport on purpose.', 'They forgot my passport.'] },
        { t: 'fill', q: 'Se me ___ las llaves.', a: 'perdieron', hint: 'perder — plural thing' },
        { t: 'fill', q: 'Se me ___ el teléfono.', a: 'rompió', hint: 'romper' },
        { t: 'mc', q: '"El aire acondicionado no funciona" means…', o: ["The air conditioning doesn't work.", 'The air conditioning is off.', "There's no air conditioning."] },
        { t: 'mc', q: 'The most polite complaint:', o: ['Disculpe, ¿sería posible cambiar de habitación?', '¡Quiero otra habitación!', 'Esta habitación es mala.'] },
        { t: 'fill', q: 'El vuelo tiene un ___ de dos horas.', a: 'retraso', hint: 'delay' },
        { t: 'order', es: 'Quisiera cambiar de habitación por favor', en: "I'd like to change rooms, please" },
        { t: 'listen', es: 'Se me olvidó la llave en la habitación', en: 'I left the key in the room' },
        { t: 'fill', q: 'Quiero ___ mi reserva.', a: 'cancelar', hint: 'to cancel' }
      ],
      speak: ['Disculpe, tengo un problema con mi habitación.', 'Se me olvidó la llave adentro.', '¿Sería posible cambiar de habitación?'],
      coach: 'Problems are the best B1 practice: they force you to explain, describe and negotiate.'
    },
    {
      title: 'Your Story & Your Plans (B1 Synthesis)', es: 'Tu historia y tus planes', canDo: 'Describe experiences, hopes and plans, and justify your opinions — the core of B1.', trick: 4,
      vocab: [['la meta', 'goal', 'Mi meta es hablar con fluidez.'], ['lograr', 'to achieve', '¡Lo vas a lograr!'], ['esperar', 'to hope; to wait', 'Espero viajar pronto.'], ['soñar con', 'to dream of', 'Sueño con vivir en la playa.'], ['me encantaría', 'I would love to', 'Me encantaría conocer Perú.'], ['la fluidez', 'fluency', 'Hablo con más fluidez.'], ['seguir + gerundio', 'to keep (doing)', 'Sigo estudiando.'], ['dejar de', 'to stop (doing)', 'Dejé de fumar.'], ['acostumbrarse a', 'to get used to', 'Me acostumbré a hablar español.'], ['valer la pena', 'to be worth it', 'Vale la pena intentarlo.'], ['el desafío', 'challenge', 'Fue un gran desafío.'], ['orgulloso / orgullosa', 'proud', 'Estoy orgulloso de mi progreso.']],
      grammar: {
        title: 'Put it all together: past, present, future',
        hack: 'A B1 speaker moves through time: what it was like (imperfecto), what happened (pretérito), what you have done (perfecto), what you do now (presente) and what you will / would like to do (futuro, condicional).',
        tables: [{ head: ['Time', 'Tense', 'Example'], rows: [['Background', 'Imperfecto', 'Antes no hablaba español.'], ['Event', 'Pretérito', 'Hace tres meses empecé a estudiar.'], ['Experience', 'Perfecto', 'He aprendido muchísimo.'], ['Now', 'Presente', 'Ahora puedo tener conversaciones.'], ['Plan', 'Futuro / ir a', 'El año que viene voy a viajar a Perú.'], ['Wish', 'Condicional', 'Me encantaría trabajar en Latinoamérica.']] }],
        examples: [['Antes no hablaba español, pero hace tres meses empecé a estudiar.', "Before, I didn't speak Spanish, but three months ago I started studying."], ['He aprendido muchísimo y estoy orgulloso.', "I've learned a lot and I'm proud."], ['Sigo practicando todos los días.', 'I keep practicing every day.'], ['Me encantaría hablar con fluidez el próximo año.', 'I would love to speak fluently next year.']],
        tip: 'This is your B1 "story". Make it personal and rehearse it until it flows.'
      },
      dialogue: [['Amiga', '¡Tu español ha mejorado muchísimo! ¿Cómo lo lograste?', 'Your Spanish has improved so much! How did you do it?'], ['Tú', 'Gracias. Hace unos meses decidí estudiar todos los días. Al principio era difícil.', 'Thanks. A few months ago I decided to study every day. At first it was hard.'], ['Amiga', '¿Y qué hiciste?', 'And what did you do?'], ['Tú', 'Cambié mi teléfono a español, escuché pódcasts y hablé con gente en intercambios.', 'I switched my phone to Spanish, listened to podcasts and talked with people in exchanges.'], ['Amiga', '¿Y ahora qué?', 'And what now?'], ['Tú', 'Sigo practicando. Me encantaría trabajar en Latinoamérica algún día.', "I keep practicing. I'd love to work in Latin America someday."], ['Amiga', 'Estoy segura de que lo vas a lograr. ¡Vale la pena!', "I'm sure you'll make it. It's worth it!"]],
      ex: [
        { t: 'mc', q: 'Antes no ___ español. (background)', o: ['hablaba', 'hablé', 'he hablado'] },
        { t: 'mc', q: 'Hace tres meses ___ a estudiar. (event)', o: ['empecé', 'empezaba', 'empiezo'] },
        { t: 'fill', q: 'Yo ___ aprendido muchísimo.', a: 'he', hint: 'haber' },
        { t: 'fill', q: 'Me ___ trabajar en Latinoamérica.', a: 'encantaría', hint: 'encantar, conditional' },
        { t: 'fill', q: 'Sigo ___ todos los días.', a: 'practicando', hint: 'practicar, gerund' },
        { t: 'mc', q: '"Vale la pena" means…', o: ["It's worth it.", "It's a pity.", 'It hurts.'] },
        { t: 'mc', q: '"Dejé de fumar" means…', o: ['I stopped smoking.', 'I let him smoke.', 'I started smoking.'] },
        { t: 'order', es: 'Me encantaría hablar con fluidez el próximo año', en: 'I would love to speak fluently next year' },
        { t: 'listen', es: 'Hace unos meses decidí estudiar todos los días', en: 'A few months ago I decided to study every day' },
        { t: 'fill', q: 'El año que viene ___ a viajar a Perú.', a: 'voy', hint: 'ir, yo' }
      ],
      speak: ['Antes no hablaba español, pero empecé a estudiar.', 'He aprendido muchísimo.', 'Me encantaría trabajar en Latinoamérica.'],
      coach: 'Record yourself telling your Spanish story today and again in a month. Hearing the difference is the best motivation.'
    }
  ];

  // Post-process: stable ids, vocab objects with gender, phase numbers.
  const U = LQ.util;
  C.units = RAW.map((u, i) => ({
    ...u, id: 'u' + String(i + 1).padStart(2, '0'), n: i + 1, phase: Math.floor(i / 5) + 1, art: u.art || 'city',
    // Question forms get their own id so «¿cuándo?» and «cuando» stay separate cards.
    vocab: u.vocab.map(([es, en, ex]) => ({ id: U.slug(es) + (es.startsWith('¿') ? '-q' : ''), es, en, ex, g: U.gender(es) }))
  }));
  C.unitById = Object.fromEntries(C.units.map(u => [u.id, u]));
  C.vocabIndex = {};
  for (const u of C.units) for (const v of u.vocab) if (!C.vocabIndex[v.id]) C.vocabIndex[v.id] = { ...v, unit: u.id };
  if (typeof module !== 'undefined') module.exports = LQ;
})(typeof window !== 'undefined' ? window : globalThis);
