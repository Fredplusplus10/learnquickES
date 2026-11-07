# LearnQuick nativo (C++20 + Qt 6)

**Estado: fase 4 de 5. El plan diario funciona de principio a fin, con voz neuronal propia, un coach que orienta y pantallas para ver tu progreso.**

- **Primera ejecución:** una bienvenida en tres pasos, siempre en inglés. Pide el nombre, por qué quieres aprender español (texto libre, mínimo 20 caracteres), para qué lo quieres, tu nivel y la duración del plan. Lo guarda en una base de datos SQLite local.
- **Hoy (*Today*):** el día del plan con sus tareas, la racha con su icono (una llama propia o el emoji que elijas, como en Snapchat), la semana, el ritmo frente al calendario, las fases y un calendario de cinco semanas. Tu porqué aparece arriba. Cada tarea se abre con un clic y se marca sola al terminarla.
- **Lecciones:** el mapa del curso (25 lecciones en 5 fases) y el reproductor de cada lección: palabras con el artículo en color, el truco de gramática con sus tablas, el diálogo, la práctica, frases para decir en voz alta y el resumen. Al terminar, las palabras entran en el repaso.
- **Repaso espaciado** (cajas de Leitner, como en la web), los **ocho entrenadores** del plan (verbos, mixto, *shadowing*, cognados, falsos amigos, el/la, pretérito o imperfecto, describir), los **controles de fase** y el **simulacro B1** con sus cinco partes. Lo que fallas vuelve al final, en una ronda de *Second try* que da más XP que antes.
- **Coach:** sugiere qué hacer y por qué, a partir de los números del propio alumno (tarjetas pendientes, días de retraso, la habilidad más floja, días sin hablar, el próximo control). Aparece en Hoy, al final de cada sesión y en su propia pantalla. Cada 3 sesiones pregunta qué le gustó y cómo le resultó, y la práctica mixta se inclina hacia lo que le gusta. El plan no cambia.
- **Práctica (*Practice*):** los nueve entrenadores en una cuadrícula, con las tarjetas pendientes de repaso.
- **Palabras (*Words*):** las 300 palabras del curso por lección, con su estado en el repaso (nueva, aprendiendo, recordada o pendiente) y un buscador que no distingue acentos ni mayúsculas, en español o en inglés.
- **Inmersión (*Immersion*):** apuntas los minutos de español que escuchas fuera de la app (pódcast, vídeo, series, lectura, música) y las conversaciones con hispanohablantes. Muestra tus horas de escucha frente a los hitos de Dreaming Spanish, recursos y consejos para series, frases para un intercambio y las misiones de poner el móvil y el ordenador en español.
- **Progreso (*Progress*):** horas en español, lecciones, palabras y racha; los minutos de cada uno de los últimos 14 días; los aciertos a la primera en cada habilidad; siete señales de un B1 que funciona, y los 16 logros de la versión web. Todo sale de lo que has hecho, nada es una estimación.
- **Ajustes (*Settings*):** modo día, noche o el del ordenador; la voz que te lee, entre seis (los personajes de los diálogos conservan la suya), y su velocidad normal y lenta, con una muestra al momento. El sol o la luna junto a la versión, abajo en la barra lateral, cambia de modo con un clic. En *Your data*, una copia de seguridad en un fichero que también abre la versión web.
- **Voz:** las frases en español suenan con una voz neuronal que va dentro de la app (Supertonic 2, sin conexión), la misma en Windows y en Mac, a un ritmo pensado para quien aprende o, si se pide, más lento. En los diálogos cada personaje tiene su voz: las mujeres, de mujer, y los hombres, de hombre. Los números se leen como palabras («305» → «trescientos cinco», «4:30» → «cuatro y media») y los nombres extranjeros como se dicen («Ohio» → «Ojáio»). Los símbolos se leen como los diría un profesor: «cansado / cansada» suena «cansado o cansada», nunca «barra». Si la voz propia fallara, la app usa la del sistema (Qt TextToSpeech).

Todo lo calcula el motor en C++ (`core/`), que reproduce exactamente el de la versión web. El mismo código se compila en Windows y en macOS, donde sale un binario universal que funciona en Apple Silicon e Intel.

La app web de `app/` sigue siendo la versión que se usa mientras se construye esta. Es también la especificación: su contenido y sus tests definen qué tiene que hacer la nativa.

## Verla sin instalar nada

1. En GitHub: **Actions → Desktop app**, abre la última ejecución en verde y baja a **Artifacts**.
2. **Windows:** descarga `LearnQuick-windows`, descomprímelo y ejecuta `bin\LearnQuick.exe`.
3. **Mac:** descarga `LearnQuick-macos`, abre el `.dmg` y arrastra LearnQuick a Aplicaciones. La app no lleva firma de Apple, así que macOS la bloquea la primera vez. Para abrirla, intenta abrirla una vez y luego ve a *Ajustes del Sistema → Privacidad y seguridad → Abrir igualmente*. Desde macOS 15, el truco de clic derecho → Abrir ya no basta.
4. **Capturas de cada plataforma:** `screenshots-macOS` y `screenshots-Windows` traen los tres pasos de la bienvenida (`welcome-1/2/3.png`) y, con el alumno de ejemplo, Hoy, el mapa de lecciones, cuatro pasos de una lección, el repaso, dos entrenadores, un control de fase, la escritura del simulacro, el coach, los ajustes, la práctica, las palabras, la inmersión y el progreso, y diez capturas en modo noche (`night-*.png`), con la bienvenida. La CI las genera con `--screenshot` y `--open`, así que se ve la versión de Mac sin tener un Mac.

## Entorno de desarrollo en Windows

Qt 6.12 da soporte oficial en Windows x86_64 a **MSVC 2022**, así que ese es el compilador que se usa. Sus binarios se llaman `msvc2022_64`.

1. Compilador (MSVC 2022 y Windows SDK), en PowerShell:

   ```powershell
   winget install --id Microsoft.VisualStudio.2022.BuildTools -e --override "--passive --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
   ```

2. Qt: instalador online de <https://www.qt.io/download-qt-installer-oss>. Pide una cuenta de Qt, que es gratuita. En *Custom installation* marca:
   - **Qt → Qt 6.12.0 → MSVC 2022 64-bit**
   - **Qt → Qt 6.12.0 → Additional Libraries → Qt Speech y Qt Multimedia.** Qt Speech da la voz desde la fase 3 y necesita Qt Multimedia, que también servirá para el micrófono en la fase 5.
   - **Qt Creator**, **CMake** y **Ninja**. Vienen marcados por defecto.

## Compilar y ejecutar

**Con Qt Creator:**

1. *File → Open File or Project…* y elige `desktop/CMakeLists.txt` dentro del repositorio.
2. Elige el kit **Desktop Qt 6.12.0 MSVC2022 64bit** y pulsa *Configure Project*.
3. Pulsa **Ctrl+R**. La primera vez se abre la bienvenida; al terminarla, la pantalla Hoy.
4. Para probar con progreso sin rellenar nada: *Projects → Run → Command line arguments* `--demo`. Con `--demo --open lesson/7/practice` se abre directamente la práctica de la lección del día.

**Desde la terminal**, en *Developer PowerShell for VS 2022* y en la raíz del repositorio:

```powershell
cmake -S desktop -B build/desktop -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/msvc2022_64
cmake --build build/desktop
ctest --test-dir build/desktop --output-on-failure
cmake --install build/desktop --prefix "$PWD\dist"
.\dist\bin\LearnQuick.exe
```

`ctest` espera `100% tests passed, 0 tests failed out of 48`:

| Test | Qué comprueba |
| --- | --- |
| `core` | El motor: fechas, festivos, planes, racha, ritmo, comprobación de respuestas, repaso espaciado, conjugador, ejercicios y sesiones (los fallos vuelven en una ronda aparte al final), puntuación del simulacro, el plan de 30 días completo y la paridad con la versión web (respuestas, los 25 verbos en 8 tiempos, el repaso, la escritura del simulacro y el mazo del alumno de ejemplo). El coach: cada sugerencia y su orden, los gustos de las reflexiones y la práctica que se inclina hacia ellos. La inmersión: el registro de minutos y conversaciones, los totales, los hitos de escucha, los 16 logros (cada uno una sola vez, el día en que se gana) y las señales de B1. La copia de seguridad: la del alumno de ejemplo exportada por la web se restaura campo por campo, la propia vuelve entera, y un fichero que no es una copia se rechaza con el motivo. También la pronunciación: los símbolos y los números leídos como palabras, los nombres respelados, la voz de cada personaje de los diálogos y los fonemas de cada frase del curso, iguales a los de espeak-ng |
| `voice` | La voz neuronal con el modelo real: tiene la voz de cada personaje, habla siempre al mismo volumen y suena igual cada vez, cada ritmo que ofrece *Settings* dura lo que dice (los de serie y los máximos), un texto largo se dice por partes con pausa, el texto llega al modelo como lo prepara el código de Supertone, una frase vacía no suena y puede decir cada frase del curso |
| `storage` | La base de datos: migraciones (también desde las de la fase 2, la 0.3.1, la 0.3.4, la 0.3.5, la 0.3.6, la 0.4.0 y la 0.4.5, sin perder nada), el perfil con el emoji de la racha, el progreso completo guardado y leído, los ajustes (un valor fuera de rango se lee como el más cercano que ofrece la pantalla), una copia restaurada sustituye perfil y progreso a la vez y, si falla a mitad, no cambia ninguno; rechazo de una base de datos de una versión más nueva. Los certificados: el nombre de la bienvenida con sus tildes y espacios, cada plantilla con el nombre, la fecha y el ID en su sitio, el aviso de que no es oficial y sin firma; un ID nuevo por certificado; sin las lecciones del nivel ni un intento aprobado no hay evidencia; la prueba se abre al terminar las lecciones, una vez al día, solo guarda intentos completos y aprueba con 70 % y 60 % en cada parte; los intentos no se pueden cambiar ni borrar; con una evaluación aprobada se emite una sola vez y no se puede cambiar ni borrar, y restaurar una copia no toca ni certificados ni intentos |
| `app` | La capa entre el motor y QML: la bienvenida; una lección terminada y sus tarjetas siguen ahí tras reiniciar; el repaso guarda las notas y marca su tarea; un entrenador marca la suya; los fallos esperan a la ronda de *Second try*, que da 8 XP por acierto y se puede saltar; el control de fase y el simulacro se guardan; cada 3 sesiones llega la reflexión, y lo que responde sigue ahí tras reiniciar; el coach apunta a la siguiente tarea y sus sugerencias abren pantallas; los ajustes se guardan al cambiarlos y volver a los recomendados respeta el modo día o noche; el alumno de ejemplo nunca escribe en la base de datos, y un modo pedido con `--theme` tampoco; lo apuntado en Inmersión sigue ahí tras reiniciar y da sus logros con aviso; Palabras lista las 300; Progreso da el objetivo, la fecha de fin y el ritmo (sin ritmo hasta pasar 3 días de estudio), los días de cada fase en el plan del alumno y las semanas solo desde que empezó, con las palabras del mazo que nunca bajan; el resumen del coach cuenta las respuestas de sus 14 días, no juzga una habilidad con menos de 8 y cuadra con sus sugerencias; la voz recomendada es la de serie y está entre las que se ofrecen; una copia guardada desde *Settings* lo devuelve todo, una de la web conserva tu porqué, y el alumno de ejemplo se puede guardar pero no sustituir |
| `certificates` | La pantalla de certificados sin ventana: el alumno de ejemplo no tiene ninguno ni puede recibirlo; sin las lecciones y la prueba no se emite nada ni se escribe nada, ni aunque llegue un intento; un suspenso se guarda y la prueba espera al día siguiente; un aprobado emite el certificado al momento, con el nombre de la bienvenida y apuntando a ese intento; uno emitido conserva el nombre con el que se emitió aunque el alumno se cambie el nombre, se guarda como SVG idéntico al original, se exporta a PDF y, si se borra su copia, se rehace igual desde la base de datos |
| `keyboard` | El teclado en pantalla, pulsado con el ratón en una escena real de Qt Quick: escribe donde está el cursor (¿, mayúsculas, é, ñ, números, €), sustituye lo seleccionado, borra (también manteniéndolo), deja el foco en la respuesta para que el teclado físico siga escribiendo, su Intro comprueba y después continúa, y en la redacción salta de línea |
| `smoke-welcome` | La app arranca sin interfaz sobre una base de datos vacía y carga la bienvenida sin avisos de QML |
| `smoke-<pantalla>` | Lo mismo con el alumno de ejemplo en cada pantalla: Hoy, lecciones, cada paso de una lección, los nueve entrenadores, un control de fase, cada parte del simulacro, el coach, los ajustes, la práctica, las palabras, la inmersión, el progreso, los certificados y la prueba de nivel. También comprueba que las pantallas usan los ajustes y la voz de la app, no una copia propia |
| `smoke-night-<pantalla>` | Ocho de esas pantallas otra vez en modo noche, que usa otros colores |

La primera configuración descarga Catch2, nlohmann/json, onnxruntime 1.23.2 y la voz Supertonic 2 (unos 290 MB en total, todo fijado por versión y hash SHA-256). Sin red, `-DLQ_BUILD_TESTS=OFF -DLQ_NEURAL_VOICE=OFF` compila solo la app, que entonces habla con la voz del sistema.

`cmake --install` copia junto al ejecutable las DLL y los plugins de Qt que necesita (incluidos el de SQLite, el de voz y el de plataforma sin ventana), además de `onnxruntime.dll` y la carpeta `voices/`, para que funcione sin Qt instalado. El despliegue de Qt exige que la ruta de `--prefix` sea absoluta (`$PWD\dist`).

La prueba de humo también se puede pasar sobre la app instalada, sin ventana y sin Qt en el equipo. Es una app de ventana, así que PowerShell solo espera a que termine con `Start-Process -Wait`:

```powershell
$env:QT_QPA_PLATFORM = "offscreen"
(Start-Process .\dist\bin\LearnQuick.exe -ArgumentList "--smoke-test", "--demo" -Wait -PassThru).ExitCode
$env:QT_QPA_PLATFORM = $null
```

Espera `0`. Cualquier otro número es un fallo (ver «Fallos conocidos»).

## Opciones de línea de comandos

| Opción | Para qué |
| --- | --- |
| `--demo` | Muestra a Cory, el alumno de ejemplo, tres semanas dentro de un plan de 60 días. No guarda nada |
| `--today 2026-11-02` | Usa esa fecha como hoy. Las capturas de CI la fijan para que sean reproducibles |
| `--data-dir <carpeta>` | Guarda la base de datos en otra carpeta. Sirve para repetir la bienvenida sin borrar tus datos |
| `--welcome-step 1-3` | Abre la bienvenida en ese paso (para capturas) |
| `--open <ruta>` | Abre esa pantalla en vez de Hoy: `lessons`, `lesson/7/practice`, `practice`, `practice/verbs`, `vocab`, `immersion`, `progress`, `checkpoint/1`, `mock/writing`, `coach`, `settings`… |
| `--theme light\|dark\|system` | Muestra la app en ese modo sin guardarlo (para capturas) |
| `--smoke-test` | Carga la interfaz y sale; falla ante cualquier aviso de QML o error de datos |
| `--require-speech` | Con `--smoke-test`: falla también si la voz propia no carga o no produce audio, o si la app no lleva el plugin de voz del sistema |
| `--screenshot <png>` | Guarda una captura de la ventana y sale |
| `--size 1440x900` | Abre la ventana a ese tamaño (mínimo 900x600), para comparar capturas a varios anchos |

## Teclado

En los ejercicios: **Intro** comprueba y pasa al siguiente; **1–9** eligen opción, artículo o nota de repaso; **Espacio** da la vuelta a la tarjeta; **Retroceso** quita la última ficha de una frase. Debajo de cada respuesta, y en la redacción del simulacro, hay un teclado completo en pantalla: letras, una fila resaltada con á é í ó ú ü ñ ¿ ¡, mayúsculas (dos toques las fijan), números y símbolos, borrar (manteniéndolo, sigue borrando), espacio e Intro. Escribe donde está el cursor sin quitarle el foco al campo, así que el teclado físico sigue funcionando a la vez. En los ejercicios, su Intro dice **Check** y luego **Continue**: la acción principal queda junto a las teclas aunque la ventana sea baja.

## Dónde se guardan tus datos

En un único fichero SQLite, `learnquick.sqlite3`:

- **Windows:** `%APPDATA%\LearnQuick\LearnQuick\`
- **macOS:** `~/Library/Application Support/LearnQuick/LearnQuick/`

Los ajustes (modo, voz y velocidades) van en el mismo fichero. Junto a él, la carpeta `certificates/` guarda una copia SVG de cada certificado ganado; si se borra, la app la rehace desde la base de datos, que es lo que cuenta.

**Copia de seguridad:** *Settings → Your data → Save a backup…* guarda todo en un fichero `learnquick-backup-<fecha>.json` (por defecto en Documentos). Es el mismo formato que la versión web, así que cada una abre las copias de la otra. *Restore from a backup…* pide confirmación y sustituye perfil y progreso de una vez. Al restaurar una copia de la web se conservan tu porqué, tus objetivos y el emoji de la racha, que la web no tiene. Los días libres y la opción de estudiar en festivos que se marcan en la web llegan con su copia y cuentan en el plan, aunque la nativa todavía no tiene pantalla para cambiarlos. Para volver a ver la bienvenida, borra ese fichero o arranca con `--data-dir` apuntando a una carpeta vacía. El esquema lleva versión (`PRAGMA user_version`) y se migra solo al abrir; una base de datos creada por una versión más nueva de la app se rechaza en vez de estropearse.

## Datos que vienen de la versión web

La versión web sigue siendo la especificación, así que dos ficheros se generan a partir de ella y no se editan a mano:

```powershell
node desktop/tools/export_course.js > desktop/content/course.json
node desktop/tools/web_reference.js > desktop/tests/data/web_reference.json
```

- `content/course.json`: el curso entero (fases, unidades con vocabulario, gramática, diálogos y ejercicios, el material de los entrenadores y el simulacro). Va dentro del ejecutable.
- `tests/data/web_reference.json`: lo que calcula el motor web (festivos de 2020 a 2040, los tres planes, el alumno de ejemplo en seis fechas con su actividad y su mazo, la comprobación de respuestas, todas las conjugaciones, el repaso espaciado, la escritura del simulacro y una copia de seguridad del alumno de ejemplo tal como la exporta la web). Los tests de paridad exigen que el motor C++ dé exactamente lo mismo.

*Repository checks* falla si alguno de los dos no coincide con lo que generaría la versión web actual.

Un tercero sale de espeak-ng, el programa cuyos fonemas aprendió la voz Piper que la app llevó de la 0.3.1 a la 0.3.3:

```powershell
pip install espeakng-loader==0.2.4
python desktop/tools/phoneme_reference.py > desktop/tests/data/phonemes_es419.json
```

- `tests/data/phonemes_es419.json`: los fonemas en español latinoamericano (`es-419`) de cada frase que la app lee en voz alta, más una lista de palabras difíciles. El test `core` exige que el fonetizador propio dé lo mismo. *Repository checks* también lo regenera y lo compara.
- `tools/phonemizer_prototype.py` es el prototipo en Python del fonetizador. Sirvió para descubrir las reglas antes de pasarlas a C++, y se mantiene igual que `core/src/speech.cpp`.

## Estructura

```text
desktop/
├── CMakeLists.txt          # Proyecto, módulo QML, pruebas de humo, instalación
├── core/                   # lq_core: el motor en C++20 puro, sin Qt
│   ├── include/lq/         # date, calendar, streak, progress, achievements, backup, demo, course, text, srs, verbs, exercises, coach, random, speech, viewing
│   └── src/
├── storage/                # lq_storage: SQLite (Qt SQL), migraciones, perfil, progreso y ajustes
├── voice/                  # lq_voice: la voz neuronal (Supertonic 2 sobre onnxruntime) y sus avisos de licencia
├── tests/                  # Catch2: motor, paridad con la web, pronunciación, voz, SQLite
│   └── data/               # web_reference.json, phonemes_es419.json
├── tools/                  # Node: datos de la versión web; Python: referencia de fonemas y prototipo
├── content/course.json     # Fases y unidades, exportadas de la versión web
├── src/
│   ├── main.cpp            # Arranque, base de datos y línea de comandos
│   ├── profilecontroller.* # El perfil para QML: si existe y guardar la bienvenida
│   ├── learnercontroller.* # El curso y el progreso: cada cambio pasa por el motor y se guarda
│   ├── learnerreports.cpp  # Progreso, inmersión y palabras para las pantallas de la fase 4
│   ├── learnerbackup.cpp   # Guardar y restaurar la copia de seguridad
│   ├── learnerviewing.cpp  # Importar el historial de Netflix y similares a Inmersión
│   ├── sessionmodel.*      # Una sesión de ejercicios en pantalla
│   ├── speechcontroller.*  # La voz: la neuronal o la del sistema (singleton Speech en QML)
│   ├── preferencescontroller.* # Los ajustes: modo, voz y velocidades (singleton Preferences en QML)
│   ├── todaymodel.*        # Todo lo que muestra la pantalla Hoy, calculado por el motor
│   └── buildinfo.*         # Versión, Qt, compilador y sistema
├── Main.qml                # Ventana: bienvenida o la app según haya perfil
├── Welcome.qml             # Bienvenida en tres pasos, en inglés
├── Shell.qml               # Barra lateral, rutas entre pantallas y avisos
├── TodayPage.qml, LessonsPage.qml, LessonPage.qml
├── TrainerPage.qml, CheckpointPage.qml, MockExamPage.qml
├── CoachPage.qml, CoachCard.qml, ReflectionCard.qml   # El coach y la reflexión cada 3 sesiones
├── PracticePage.qml, VocabularyPage.qml, ImmersionPage.qml, ProgressPage.qml   # Fase 4
├── SettingsPage.qml        # Modo día o noche, tu voz y su velocidad, la copia de seguridad
├── ExerciseSession.qml     # Los 11 tipos de ejercicio, con teclado
├── SessionSummary.qml, PageHero.qml, Breadcrumb.qml, ScrollPage.qml
├── Icon.qml, SpeakButton.qml, SpanishWord.qml, SpanishKeyboard.qml
├── Card.qml, Chip.qml, ChoiceCard.qml, PrimaryButton.qml, InputField.qml
├── Theme.qml               # Colores del design kit, de día y de noche (singleton QML)
├── images/
└── platform/               # Info.plist, iconos de macOS y Windows
```

## Decisiones

- **Tres capas.** `lq_core` (C++20 sin Qt) contiene las reglas; `lq_storage` guarda; `src/` las expone a QML. QML solo describe pantallas: no calcula nada.
- **Fechas como días civiles.** El motor usa `std::chrono::sys_days`, un número de días sin hora ni zona horaria, así que un cambio de hora no puede mover un día de estudio.
- **El motor web es la referencia.** Los tests de paridad comparan el motor C++ con lo que produce la versión web, incluido el generador aleatorio del alumno de ejemplo. Si alguna vez difieren, uno de los dos tiene un error.
- **El curso se lee en el motor.** `lq_core` carga `course.json` con nlohmann/json, así que la construcción de ejercicios, la comprobación y la puntuación se prueban sin Qt.
- **El progreso se guarda entero en cada cambio,** en una transacción. Son unos cientos de filas como mucho: más simple y más seguro que seguir cambios sueltos, y un corte a mitad deja intacta la copia anterior.
- **Voz neuronal propia, no la del sistema.** Las voces del sistema suenan robóticas, y las buenas no están a nuestro alcance: las voces naturales de Windows 11 solo las usa el Narrador, no las apps. Por eso la app lleva su propia voz:
  - **Supertonic 2** (Supertone): un modelo con diez voces, 44,1 kHz, unos 260 MB. Licencia BigScience OpenRAIL-M: permite el uso comercial y la redistribución si la licencia viaja con el modelo y se respetan sus restricciones de uso (anexo A). La app lleva seis voces. Desde la 0.4.5, M2 (Julián) narra y es «Tú», porque es la más clara y el curso escribe al alumno en masculino («estoy nervioso»). F3 (Clara) es la voz de todas las mujeres, empezando por Lucía. Los demás hombres alternan M5 y M4 (`lq::voiceFor`), así que ninguno suena como «Tú». En 36 frases del curso, al ritmo de la app, Whisper falló el 1,5 % de las palabras con M2, el 3,0 % con F3 y M4, el 3,8 % con F1, el 4,1 % con M5 y el 6,8 % con M1, que salió del reparto. F4 y F5 fallaban el 11 %, comiéndose palabras enteras. Cada personaje sale en su propia unidad, así que se prefirió la claridad a la variedad.
  - **Elegida midiendo, no a oído.** 26 voces candidatas (Piper de México, Argentina y España; Supertonic 2 y 3) leyeron 12 frases del curso. Whisper las transcribió para contar errores por palabra, UTMOS puntuó la naturalidad de 1 a 5 y se midió el tono para saber si suenan a mujer o a hombre. La voz anterior, Piper `es_MX-ald-medium`, tenía un 13 % de errores y 3,09 de naturalidad, y «Soy de Ohio» se oía «Soy Leo yo». Supertonic 2 F3: 1 % y 3,95; M4: 0 % y 3,94. En 40 palabras sueltas del vocabulario, F3 acertó 38 y la voz anterior 15. Supertonic 3 leía «Ohio» como «Oyo» y pesa unos 120 MB más.
  - **Acento:** sesea, como en Latinoamérica. «Caza» suena igual que «casa», sin la *z* española (medido en el espectro de seis parejas así).
  - **El motor está escrito en C++** (`voice/src/supertonic.cpp`) a partir del paquete de Python de Supertone. Da el mismo audio que Python (correlación 1,00000 en las 12 frases) porque también saca el ruido de partida como numpy. **onnxruntime 1.23.2** lo ejecuta en la CPU, sin conexión: una frase tarda unas 0,07 veces lo que dura, y el modelo carga en 0,6 s. Es la última versión de onnxruntime con binario universal para Mac (Apple Silicon e Intel).
  - **Un ritmo para quien aprende.** A su paso, las voces son demasiado rápidas para un alumno. Pedirle al modelo que vaya más despacio solo alarga las pausas y emborrona las palabras: a 0,9 sigue articulando a 5,2 sílabas por segundo y Whisper falla el 6 % de las palabras, y con 0,8 entendió 6 de 12 frases. Por eso la app planifica la frase a su paso natural y la alarga sin cambiar el tono.
  - **Todas las voces al mismo ritmo desde la 0.4.5** (`lq::voiceStretch`). Cada voz tiene su propio paso, medido en las 36 frases: de 4,0 sílabas por segundo (M5) a 5,2 (F4). Hasta la 0.4.0 todas se alargaban 1,2 veces, así que Tomás (M5) se quedaba en 3,3 sílabas por segundo y parecía frenarse, mientras Julián iba a 4,0. Ahora el ritmo se mide contra una referencia común, 4,8 sílabas por segundo (1×, el paso medio de las voces y el de Julián), y cada voz se alarga lo justo para llegar: con el ritmo por defecto, de 3,88 a 4,01 sílabas por segundo en las seis. Los ritmos recomendados siguen el consejo habitual para principiantes, del 0,75× al 0,85× de la velocidad normal de la voz: 0,83× (×1,2, 4,0 sílabas por segundo) el normal y 0,74× (×1,35, 3,6) el lento. Hasta la 0.4.0 el lento era 0,63× (×1,6) y terminaba de estirar el sonido con WSOLA, el método del eco metálico; el nuevo se queda casi entero en los latentes. Si una voz es más lenta que el ritmo pedido (M5 a 1×), se le pide al modelo que hable un poco más rápido.
  - **Pausas entre frases, sin cortar la línea.** Las pausas se alargan a 0,6 s con el ritmo normal (lo aconsejado para un nivel intermedio, de 0,5 a 0,8 s) y a 1,5 s con el lento (lo aconsejado para principiantes). La línea se sigue diciendo entera y después se alargan sus silencios: tantos de los más largos como huecos entre frases tiene. Decir cada frase por separado subía los errores (en las 36 frases, M2 del 1,5 % al 3,8 % y F3 del 3,0 % al 4,5 %; un «¿Y tú?» suelto salía «titulado»), y separar solo una frase inicial corta como «¡Hola!» también (F3, del 3,6 % al 5,5 % en las 48 líneas de diálogo que empiezan así).
  - **Menos robótica desde la 0.3.6.** Hasta la 0.3.5 se estiraba la onda ya hecha (WSOLA), que deja un eco metálico. Ahora se estiran los *latentes*, lo que el modelo entrega al vocoder antes de convertirlo en sonido: cada paso agrupa 6 tramas de 24 valores, que se separan, se interpolan en el tiempo y se vuelven a agrupar. Hasta 1,25 se estiran solo los latentes; un ritmo más lento termina de estirar el sonido con WSOLA. En 36 frases del curso (naturalidad UTMOS, de 1 a 5): a 1,2, F3 sube de 3,87 a 3,95 y M4 de 3,73 a 3,78, con los mismos errores en F3 (3,0 %) y tres palabras más de 265 en M4 (5,6 % frente a 4,5 %). A 1,6, la mezcla da 3,64 en F3 (3,50 antes) y 3,49 en M4 (3,50 antes); estirar solo los latentes hasta 1,6 daba 3,51 y 3,35, y se descartó. Dieciséis pasos de flujo en vez de ocho apenas mejoraban (3,98 frente a 3,95) con el doble de cálculo.
  - **Una frase suena igual cada vez,** porque la semilla del ruido es fija. Importa cuál: Whisper transcribió 120 palabras del curso con ocho semillas y acertó entre 197 y 220 de 240. Desde la 0.4.5 cada voz tiene la suya: se probaron las semillas 0 a 3 en 40 frases del curso y se comprobó la mejor en otras 36. F1 pasó del 5,3 % al 3,8 % de palabras falladas, M4 del 5,6 % al 3,0 % y M5 del 5,6 % al 4,1 %; F3, M1 y M2 se quedan con la de antes, que ya era su mejor.
  - **Sonido para aprender (0.4.5).** Un filtro quita lo que hay por debajo de 80 Hz y otro sube 1,5 dB alrededor de 3,2 kHz, donde se distinguen las consonantes. En las 36 frases no cambian ni los errores ni la naturalidad (M2 3,70 frente a 3,71, F3 3,98 igual). El compresor suave que también aconsejan las guías (2,5:1) restaba de 0,11 a 0,18 de naturalidad en todas las voces y se descartó. El volumen estable sale de igualar cada frase: su voz a −18 dBFS, sin picos por encima de −1 dBFS. Hasta la 0.4.0 se igualaba el pico de cada frase, y Clara sonaba unos 5,5 dB más fuerte que Julián. El tono no se sube: desplazarlo añade artefactos por una mejora de 2 a 5 Hz.
  - **Números y nombres pasan a palabras** antes de llegar al modelo (`lq::speechText`). Con cifras dudaba («305» salía «300-105»), y «Ohio» se respela «Ojáio».
  - **Fonetizador propio** (`core/src/speech.cpp`). Lo necesitaba la voz Piper de la 0.3.1 a la 0.3.3, que lee fonemas de espeak-ng; espeak-ng es GPL-3.0 y obligaría a abrir la app entera. Da exactamente lo mismo que espeak-ng en las 561 frases del curso. Supertonic lee texto, así que ya no interviene al hablar.
  - **Los símbolos se leen como palabras** con cualquier voz: «a / b» → «a o b» (o «u» ante *o-* y *ho-*), «él/ella/usted» → «él, ella o usted», «+» → «más», paréntesis fuera, «…» → pausa.
  - **La voz del sistema queda de respaldo,** con su preferencia por voces de México, EE. UU., el resto de Latinoamérica y por último España. Sin ninguna voz, los ejercicios de escuchar pasan a ser de ordenar la frase con la traducción a la vista.
  - La síntesis corre en un hilo aparte y el audio sale por Qt Multimedia (`QAudioSink`), así que la interfaz no se bloquea. Las pruebas de humo y las capturas no hablan.
- **El coach orienta; el plan no cambia.**
  - Sus reglas están en `core/src/coach.cpp`, con tests, no en QML. Por orden de prioridad: repaso acumulado (15 tarjetas o más), retraso de 2 días o más, un control en los próximos 3 días del plan, la habilidad con menos del 70 % de aciertos en dos semanas (con al menos 8 respuestas), una lección por debajo del 60 %, 4 días sin hablar, cómo le resultó la última sesión, un entrenador que le gusta cuando ya ha terminado el día, ventaja de 2 días o más, y la siguiente tarea del plan.
  - Cada sugerencia explica el porqué con sus números y abre su pantalla. Al terminar una sesión no manda de vuelta a la misma actividad.
  - Nunca cambia el plan ni lo que pide cada día. El plan es el compromiso con el alumno: si el coach lo reescribiera, nadie sabría qué toca mañana.
  - **Cada 3 sesiones** (lecciones y entrenadores; los controles y el simulacro son exámenes y no cuentan) pregunta qué le gustó, cómo le resultó (*Too easy*, *Just right*, *Too hard*) y deja una nota opcional de 280 caracteres. *Not now* lo aplaza otras 3. Desde la pantalla del coach se le puede decir en cualquier momento.
  - **Lo que le gusta inclina la práctica mixta, sin excluir nada.** Se generan 30 ejercicios candidatos y se eligen 10 con un peso por tipo que sale de las 6 últimas reflexiones: entre 0,5 y 1,5, nunca 0, para que ningún tipo desaparezca. Tras *Too easy* pesan más los de escribir y construir frases; tras *Too hard*, los de elegir.
  - Se guarda en SQLite (esquema 3: tablas `reflection` y `session_count`).
- **Los fallos vuelven al final, en una ronda aparte.** Antes, cada fallo se añadía a la cola en cuanto ocurría y el contador crecía a mitad de sesión. Ahora la primera ronda conserva su longitud y, al terminarla, una pantalla presenta el *Second try* con cuántas respuestas quedan por arreglar. Cada acierto ahí da 8 XP (5 en la versión web), por debajo de los 10 de acertar a la primera, para que fallar a propósito nunca compense. La puntuación sigue contando solo los primeros intentos, y la ronda se puede saltar (*Finish now*).
- **La racha lleva icono propio.** Por defecto, una llama de dos tonos (`images/streak.svg`; `StreakIcon.qml` dibuja los mismos trazos, así que se ve nítida a cualquier tamaño). El alumno puede cambiarla por cualquier emoji, como en Snapchat: uno de los 24 sugeridos o el que escriba o pegue desde el panel de emojis del sistema. `lq::firstEmoji` se queda con el primer emoji entero (banderas, tonos de piel, familias unidas con ZWJ, teclas como 1️⃣). Se guarda en el perfil (esquema 4: columna `streak_emoji`). No se sugieren banderas porque Windows no las dibuja: muestra dos letras.
- **Ajustes guardados al momento, sin botón de guardar.** Cada cambio pasa por `lq::normalized` (`storage/src/settings.cpp`): el modo es `light`, `dark` o `system`; la voz, una de las que ofrece la pantalla (`lq::voiceChoices`); las velocidades van en pasos de 0,05 dentro de su rango (normal de ×1,0 a ×1,4, lenta de ×1,15 a ×1,8; la pantalla las muestra como velocidad, de 1,00× a 0,56×), y la lenta queda al menos 0,15 por encima de la normal para que siempre se note. Así un valor tocado a mano en la base de datos se lee como uno que la pantalla puede mostrar. Se guardan en una tabla clave-valor (esquema 5: tabla `setting`), para que un ajuste nuevo no necesite migración.
  - **Los diálogos no se tocan.** La voz elegida lee todo lo demás (palabras, ejemplos, ejercicios); cada personaje conserva la suya para que siempre se sepa quién habla.
  - **Seis voces,** de la más clara a la menos, con sus errores y su naturalidad en las 36 frases (0.4.5): Julián (M2, recomendada) 1,5 % y 3,71; Clara (F3) 3,0 % y 3,98, la más natural; Mateo (M4) 3,0 % y 3,84; Inés (F1) 3,8 % y 3,82; Tomás (M5) 4,1 % y 3,79; Hugo (M1) 6,8 % y 3,54. F2, F4 y F5 no se ofrecen (de 5,3 % a 10,5 % de errores, o menos naturales). Cada voz extra pesa unos 410 KB. La 0.4.5 borra la voz y los ritmos guardados (esquema 7), porque el ritmo ya no significa lo mismo, y cada alumno vuelve a los recomendados.
- **Modo noche hecho con tokens, no con un filtro.** `Theme.qml` define cada color dos veces, de día y de noche, y ninguna pantalla escribe un color a mano: el texto sobre naranja usa `inkOnAccent`, los bordes `edge`, los avisos `toast`… Por defecto sigue al sistema (`Qt.styleHints.colorScheme`). Una propiedad QML que empieza por «on» y mayúscula se toma por un manejador de señal; por eso se llama `inkOnAccent` y no `onAccent`.
- **Una copia de seguridad en el formato de la web** (`core/src/backup.cpp`), no uno propio: un alumno puede pasar de una versión a otra sin perder nada. El fichero es `{app, exported, state}` con el estado como lo guarda la web. Lo que solo tiene la nativa (el porqué, los objetivos, el emoji de la racha, las reflexiones, los intentos de cada control) viaja en `state.desktop`; la web conserva las claves que no conoce al guardar (`{...base, ...raw}` en su `migrate`), así que no se pierde en el viaje de ida y vuelta. Al leer se hace lo mismo que la web: falta algo y se toma el valor por defecto; una lección que el curso no tiene se descarta. La restauración escribe perfil y progreso en una sola transacción (`lq::restoreLearner`): o cambia todo o nada.
- **Logros y señales de B1, solo con lo registrado** (`core/src/achievements.cpp`). Los 16 logros y sus condiciones son los de la web. Cada uno se apunta una vez, con la fecha del día en que se gana, y avisa con un mensaje. Al abrir la 0.4 por primera vez, los que el alumno ya tenía ganados se apuntan sin un aviso por cada uno. Las siete señales de B1 (las 25 lecciones, el pretérito y el imperfecto al 85 %, los verbos al 80 %, 200 palabras recordadas, 50 horas de escucha, 5 conversaciones y el simulacro al 70 %) orientan; el certificado es el DELE B1 del Instituto Cervantes. Se guardan en SQLite (esquema 6: tablas `input_log`, `exchange_log`, `mission` y `achievement`, y la columna `talk_minutes`; el mismo esquema añade al perfil `skip_holidays` y `days_off`, que llegan con las copias de la web).
- **La voz del narrador** (*Settings*). Muestra las seis voces que de verdad trae el motor, con su marca de elección y un botón ▶ para escucharlas sin elegirlas. «Recommended» va en la voz con la que empieza un alumno nuevo (`Preferences.defaultVoice`) y explica el motivo: con ella, un reconocedor de voz entendió más palabras del curso. Los diálogos conservan su reparto.
- **Inmersión y coach** (`ImmersionPage.qml`, `CoachPage.qml`). Inmersión abre con los minutos de hoy frente al objetivo del plan; debajo, el registro (tipo, duración y nota) con su historial y la opción de borrar, y al lado los hitos de horas de escucha (una referencia, no un nivel MCER: la isla y las conversaciones cuentan como tiempo en español, no como escucha). Si no hay nada registrado lo dice. El coach sigue usando solo sus reglas fijas: cada sugerencia lleva su motivo y su botón, y al lado aparecen los números en que se basa (`LearnerController::coachReport`), con la misma ventana de 14 días y el mismo mínimo de 8 respuestas que el motor, de modo que «aún no hay datos suficientes» no se confunde con «va mal».
- **Lecciones y cada lección** (`LessonsPage.qml`, `LessonPage.qml`). El catálogo muestra cuántas de las 25 están hechas, un botón para seguir (la lección de hoy en el plan, o la primera sin hacer) y las cinco fases con sus lecciones en el orden y con los índices de siempre: tres por fila con espacio, dos en medio, una en ventana estrecha; títulos en dos líneas, con el título entero al pasar el ratón o con el foco si no caben; la mejor nota real y el control de fase como fila aparte. Cada lección tiene migas de pan, su título con la ilustración, los pasos con la duración del plan (unos 20 min) y, al lado, sus objetivos marcados según avanza el alumno y sus palabras. Las palabras no se muestran durante la práctica, porque darían las respuestas.
- **Práctica y Palabras** (`PracticePage.qml`, `VocabularyPage.qml`). Práctica abre con el montón de repaso y agrupa los nueve entrenadores por lo que trabajan. Cada uno dice si el plan de hoy lo pide o ya está hecho, si es la sugerencia del coach y el acierto a la primera de su destreza en los 14 días del coach, con el mismo mínimo de 8 respuestas; hablar y describir los comprueba el alumno, así que no llevan nota. No hay una tabla de rondas por entrenador: la cifra sale de lo que el motor ya guarda por destreza (`answer_tally`). Los falsos amigos tienen destreza propia (`falsefriends`), también en la versión web, así que cada entrenador muestra la suya y el coach los juzga por separado. Las respuestas anteriores al cambio siguen contando como `cognates`; el coach solo mira 14 días, así que en dos semanas las cifras salen limpias. El logro *Cognate Hunter* suma las dos destrezas, para que nadie lo gane ni lo pierda por el cambio. Palabras abre con cuatro cifras (para hoy, aprendiendo, recordadas y sin estudiar) que filtran la lista. «Recordada» es la caja 4 o más del repaso (`lq::kRememberedBox`): vuelve dentro de 8 días o más.
- **Series en español por nivel** (`content/practice.js` → `course.json` `immersion.series`, `lq::seriesLevel`). Cinco series de España con dónde verlas en España (noviembre de 2026): *Extra en español* (A1, hecha para alumnos), *Aquí no hay quien viva* (A2), *Machos alfa*, *Con el culo al aire* y *La que se avecina* (B1, la última la más difícil). Primero sale el nivel del alumno: el de la bienvenida (A2+ cuenta como A2), que sube al aprobar con un 70 % el control de la fase 2 (A2) y el de la fase 4 (B1). Nunca baja, y la tarjeta dice qué abre el siguiente. Los otros niveles están a una pestaña, los de arriba marcados «para más adelante». La web aplica la misma regla. Las cinco tienen acento castellano y vosotros, que el curso no enseña, y la tarjeta lo dice.
- **Lo que ves en Netflix cuenta como escucha** (`core/src/viewing.cpp`, `src/learnerviewing.cpp`, tarjeta *Import what you watched* en Inmersión). Ningún servicio de streaming deja que otra app lea tu cuenta: Netflix cerró su API pública en 2014, y HBO Max, Prime Video y Disney+ nunca la tuvieron. Pedir la contraseña daría a la app la cuenta entera, así que no se hace. El alumno descarga su historial y lo abre aquí, y el fichero no sale del ordenador. Se leen tres formas:
  - el CSV de Netflix *Viewing activity → Download all* (`Title,Date`, con la fecha como la escribe el país de la cuenta; el orden día/mes se deduce del propio fichero y, si ninguna fecha lo aclara, del formato del sistema);
  - el `ViewingActivity.csv` de la descarga de datos personales de Netflix, con perfil, hora de inicio y duración;
  - cualquier CSV con columnas `Title` y `Date` y, si se sabe, `Minutes`.

  Ningún fichero dice en qué idioma estaba el audio, así que no se marca nada por defecto: el alumno marca las series que vio en español (las importadas antes vuelven marcadas). Cada título y día entra una vez en el registro como *series*, en el día en que se vio. Suma horas de escucha y tiempo en español, pero **no da XP**: así nunca arregla ni alarga una racha a posteriori. Al borrarlo se quitan sus minutos y nada más, también en la web. Solo cuentan los días desde el inicio del plan, salvo que el alumno pida los anteriores, y nunca días futuros. Se dejan fuera los tráileres, los avances y lo reproducido menos de 2 minutos; sin duraciones, cada episodio cuenta los minutos que diga el alumno (40 por defecto). Se guarda en SQLite (esquema 9: columna `input_log.imported`) y en la copia de seguridad (`"imported": true`).

  La guía para el alumno, con los pasos de Windows y de Mac, está en [`docs/NETFLIX-GUIDE.md`](../docs/NETFLIX-GUIDE.md) y, la misma, dentro de la tarjeta (*Step-by-step guide*, abierta en el sistema del alumno). En Mac, la primera lectura de Descargas hace que macOS pida permiso (`NSDownloadsFolderUsageDescription` en `Info.plist` explica para qué), así que allí la app no mira sola hasta que el alumno pulsa *Look in Downloads*; en Windows mira al abrir Inmersión y cada vez que la app vuelve al frente.

  Para que sea fácil: la tarjeta da tres pasos con un botón directo a la actividad de Netflix. Al volver a la app (cuando recupera el foco), busca en Descargas un historial de las dos últimas semanas (`NetflixViewingHistory*.csv`, o la carpeta `netflix-report` ya descomprimida) y ofrece abrirlo con un clic. Deja de ofrecerlo cuando sus días ya están importados (`LearnerController::downloadedViewing`). También se puede arrastrar el fichero a la tarjeta. Las series y películas hechas en español se marcan por su título de Netflix en español o en inglés (`lq::madeInSpanish`: *La casa de papel* o *Money Heist*), salen primero y un botón las marca todas. Nunca se marcan solas, porque Netflix pone por defecto el doblaje del idioma del perfil.
- **Entrenadores, controles y simulacro** (`TrainerPage.qml`, `CheckpointPage.qml`, `MockExamPage.qml`). Llevan las mismas migas de pan (`Breadcrumb.qml`) y cabecera que las lecciones. Con ventana ancha, al lado del ejercicio van el historial del alumno en esa destreza y la chuleta, plegada hasta que se pide porque contiene las respuestas; el control muestra las lecciones de su fase y el simulacro el estado de cada parte.
- **Progreso, de lo general al detalle** (`ProgressPage.qml`). Arriba: el objetivo del plan, la fecha en que acaba a un día del plan por día de estudio y el ritmo (días de estudio por delante o por detrás; con menos de 3 días de estudio pasados dice *Not enough data*), las cinco fases con los días que ocupan en ese plan, el tiempo de español por semana, las palabras del mazo al final de cada semana (de `Card.added`), la racha, el calendario y los últimos logros. La pronunciación muestra un aviso de que no hay puntuaciones: la app aún no la mide, solo cuenta las frases dichas en voz alta. Debajo, *In detail*, sigue todo lo que había: totales, minutos de los últimos 14 días, acierto por destreza, señales de B1, certificados y los 16 logros. La racha y el calendario son los mismos componentes que en Hoy (`StreakCard.qml`, `CalendarCard.qml`).
- **Certificados A1, A2 y B1 de LearnQuick, no oficiales** (`storage/src/certificates.cpp`, `CertificatesPage.qml`). Son reconocimientos internos de preparación, no del MCER ni del DELE, y lo dicen la pantalla, el certificado y el PDF. Llevan el nombre que el alumno eligió en la bienvenida (sin volver a pedirlo, con sus tildes y espacios, en NFC), la fecha y un ID `LQ-<nivel>-<año>-<12 hex>`. Solo se emiten al aprobar la evaluación de ese nivel; los días, las lecciones y los controles de fase nunca bastan. Esas pruebas de nivel están en `content/assessments.json` (escritas a mano, aparte de `course.json`) y en `lq/assessment.h` (`AssessmentPage.qml`, ruta `assessment/<nivel>`): lectura, comprensión auditiva y gramática y vocabulario, todo corregido por la app, sin autoevaluación. Se abren al terminar las lecciones del nivel (fase 1 para A1, fases 1 y 2 para A2, las 25 para B1), una vez al día, y se aprueban con un 70 % en total y al menos un 60 % en cada parte; al aprobar se emite el certificado. Cada intento saca una de dos lecturas, uno de dos audios y 12 de 20 preguntas, siempre las mismas en el día para ese alumno, así que salir a mitad no enseña preguntas nuevas. No se marca ninguna respuesta mientras dura, la opción correcta no llega a QML y nada cuenta como práctica. Cada intento terminado se guarda en `assessment_result` (esquema 10), que no se puede cambiar ni borrar; el certificado apunta al intento en que se basa. Un certificado emitido no se sobrescribe: uno por alumno, nivel y revisión del curso, y la base de datos rechaza cambiarlo o borrarlo (esquema 8: tabla `certificate_award` con dos disparadores). Se guarda como SVG tal cual y se exporta a PDF vectorial A4 con Qt SVG. Para dibujarlo, la app cambia cada lista de fuentes (`Georgia, Times New Roman, serif`) por la primera instalada, porque Qt SVG no lee listas; el SVG guardado conserva la lista para los navegadores. Se abre desde *Progress* y *Settings → Your data*.
- **La bienvenida siempre va en inglés,** porque el alumno es angloparlante. El porqué se guarda tal como lo escribe y se muestra como texto plano, nunca como HTML.
- **El target se llama `learnquick` y el ejecutable sale en `build/bin/`.** Así no choca con la carpeta `LearnQuick/` que genera el módulo QML. El nombre visible sigue siendo LearnQuick (`OUTPUT_NAME`).
- **Las pruebas de humo y las capturas fallan ante cualquier aviso de QML.** Es el equivalente a los «0 errores de consola» de la versión web. La CI las pasa sobre la compilación y sobre la app ya empaquetada. La segunda se ejecuta sin rastro del Qt de la máquina de la CI, así que demuestra que el paquete lleva todo lo necesario, incluidos los plugins de SQLite, de voz y de plataforma.
- **CI (`.github/workflows/desktop.yml`):**
  - Solo se lanza cuando cambia `desktop/`.
  - En Windows usa Qt 6.10.3, porque aqtinstall 3.3, la herramienta que descarga Qt en la CI, todavía no sabe bajar la 6.11 o posterior para Windows. En macOS usa Qt 6.12.0.
  - El `.dmg` lleva firma ad hoc, sin notarizar.
- **Versión mínima de Qt: 6.8.**

## Fallos conocidos

Errores que ya nos han pasado, con su causa, para no repetirlos.

### QML creaba sus propios ajustes y su propia voz (0.3.6)

- **Síntoma:** con `--theme dark`, las capturas de la CI salían en modo día, y *Settings* marcaba «Like my computer». En la app de verdad, los cambios de *Settings* no se guardaban y la voz elegida no llegaba a la narración.
- **Causa:** `Preferences` y `Speech` son singletons de QML (`QML_SINGLETON`) con una función `create()` que devuelve el objeto que prepara `main()`. Pero las dos clases tenían constructor por defecto, y cuando existen ambos, Qt prefiere el constructor: QML fabricaba una segunda copia de cada una, sin base de datos y sin la voz configurada. Las pruebas no lo veían porque usan los controladores desde C++, sin QML.
- **Arreglo:** los constructores piden el `parent` explícitamente, así que no son constructores por defecto y Qt usa `create()`.
- **Prevención:** un `static_assert` en cada clase impide volver a añadir el constructor por defecto, y la prueba de humo falla si el singleton que ve QML no es el objeto de `main()`.
- **Regla:** un singleton de QML que se crea en C++ no tiene constructor por defecto.

### Las voces no hablaban igual de rápido ni igual de fuerte (0.4.5)

- **Síntoma:** Tomás «se frena mucho», y unas voces sonaban más fuertes que otras.
- **Causa:** el ritmo se aplicaba igual a todas las voces (×1,2), pero su paso natural va de 4,0 a 5,2 sílabas por segundo; Tomás, la más lenta, se quedaba en 3,3. Y cada frase se normalizaba por su pico, no por su volumen: una voz con picos altos (Julián) quedaba unos 5,5 dB por debajo de otra con picos bajos (Clara).
- **Arreglo:** cada voz se alarga según su propio paso (`lq::voiceStretch`) para llegar al ritmo pedido, y cada frase se lleva a −18 dBFS de voz, con el pico como límite.
- **Prevención:** una prueba de voz exige que todas las voces ofrecidas digan las mismas frases en un ±10 % del mismo tiempo (miden ±3 %), y otra que el volumen de la voz quede en −18 dBFS.
- **Regla:** cada voz se mide por separado; nunca se da por hecho que dos voces comparten ritmo o volumen.

### La prueba de la app empaquetada usaba el Qt de la CI (0.3.2)

- **Síntoma:** en la CI, la prueba de humo de la app ya empaquetada pasaba en Windows y en macOS. En un PC sin Qt instalado, la misma prueba sin ventana (`QT_QPA_PLATFORM=offscreen`) se cerraba al arrancar.
- **Causa:** la acción que instala Qt en la CI (`install-qt-action`) deja la máquina preparada para usar ese Qt: define `QT_PLUGIN_PATH` y `QML2_IMPORT_PATH` y añade la carpeta `bin` de Qt al `PATH`. Cuando a la app empaquetada le faltaba algo, lo encontraba ahí. Y faltaba algo: el despliegue de Qt (`windeployqt`, `macdeployqt`) solo copia el plugin de plataforma con ventana (`qwindows`, `qcocoa`). El plugin sin ventana, `qoffscreen`, el que usan las pruebas y las capturas, salía del Qt de la CI. La prueba demostraba menos de lo que decía.
- **Arreglo:**
  - `cmake --install` copia también `qoffscreen`: en `plugins/platforms/` en Windows y en `LearnQuick.app/Contents/PlugIns/platforms/` en macOS, donde además se le indica dónde están los frameworks de Qt.
  - El paso de la CI borra esas variables y quita el Qt de la CI del `PATH` antes de probar.
- **Prevención:** la CI arranca la app una vez con `QT_DEBUG_PLUGINS=1`, que hace que Qt anote cada plugin que busca y carga. Falla si `qoffscreen` no aparece o si alguna ruta apunta a la instalación de Qt de la CI.
- **Regla:** una prueba del paquete se ejecuta en las condiciones del alumno, sin nada instalado aparte de la app.

### Una propiedad QML con el nombre de una propiedad de Item (fase 3)

- **Síntoma:** la compilación terminó bien en Windows y en macOS, pero fallaron todas las pruebas de humo (27 de 29 tests) con este aviso en cadena:

  ```text
  Final member right is overridden in class Option. The override won't be used.
  ExerciseSession.qml:153:9: Cannot override FINAL property
  LessonPage.qml:480:13: Type ExerciseSession unavailable
  Shell.qml:186:9: Type LessonPage unavailable
  Main.qml:41:9: Type Shell unavailable
  ```

- **Causa:** el botón de opción de los ejercicios (`Option`, en `ExerciseSession.qml`) declaraba `property bool right` para marcar la respuesta correcta. Todo componente visual de QML deriva de `Item`, y `Item` ya tiene una propiedad `right`: es la línea de anclaje del borde derecho, la que se usa en `anchors.left: otro.right`. Qt la marca como **FINAL**, es decir, ningún tipo derivado puede volver a declararla. Cuando el motor de QML encuentra la redeclaración, rechaza el componente entero, y con él cada pantalla que lo contiene: `ExerciseSession` → `LessonPage` → `Shell` → la ventana principal. Por eso falló hasta la bienvenida, que no tiene ejercicios.
- **Por qué no lo vio el compilador:** el C++ compila igual. Los ficheros QML se cargan y se comprueban al arrancar la app, así que el error solo aparece al ejecutarla. Eso es justo lo que hacen las pruebas de humo.
- **Arreglo:** la propiedad se llama ahora `correct`.
- **Prevención:** `scripts/validate_repository.py` rechaza cualquier `property <tipo> <nombre>` de `desktop/*.qml` cuyo nombre sea una propiedad de `Item` (`left`, `right`, `top`, `bottom`, `x`, `y`, `width`, `height`, `state`, `visible`, `enabled`, `data`…; la lista completa está en el script). Corre en *Repository checks*, que tarda segundos, en vez de esperar los 15 minutos de la compilación.
- **Regla:** las propiedades propias llevan un nombre que diga qué significan en este programa (`correct`, `reachable`, `isFinal`), no palabras genéricas de posición, tamaño o estado. Lo mismo vale para las palabras reservadas de QML: `final` tampoco sirve como nombre de propiedad, y por eso la pantalla de lecciones usa `isFinal`.

### Windows trae su propio onnxruntime.dll, más antiguo (voz)

- **Hallazgo:** Windows 11 incluye `C:\Windows\System32\onnxruntime.dll`, la versión 1.17, para sus propias funciones de IA. La app usa la 1.23.2.
- **Riesgo:** Windows busca primero las DLL en la carpeta del ejecutable y después en System32; las carpetas del `PATH` van detrás. Si nuestra copia no está junto al `.exe` (por ejemplo, si un test confía en el `PATH`), Windows carga la 1.17. Esa versión no conoce la API 23 que piden nuestras cabeceras, y sin protección la app se cerraría al hablar por primera vez.
- **Prevención:**
  - CMake copia `onnxruntime.dll` junto a cada ejecutable que la usa: la app, en la compilación y en la instalación, y `lq_voice_tests`.
  - `voice/src/ort.h` pide la API a mano (`ORT_API_MANUAL_INIT`) y comprueba que existe antes de usarla. Si falta, la voz propia se da por no disponible y la app sigue con la del sistema.
  - `--require-speech` sintetiza una palabra con la app ya empaquetada, así que la CI falla si la DLL no viaja con ella.

### Qt TextToSpeech no se encuentra aunque esté instalado (fase 3)

- **Síntoma:** `cmake` se detiene en `find_package` con `Qt6TextToSpeech could not be found because dependency Qt6Multimedia could not be found`.
- **Causa:** el módulo de voz de Qt depende de Qt Multimedia. La CI instala con aqtinstall solo los módulos que se le piden, y solo se le pedía `qtspeech`.
- **Arreglo:** `modules: qtspeech qtmultimedia` en `.github/workflows/desktop.yml`. En una instalación local, marca los dos módulos en el instalador de Qt.

## Fases

Cada fase entrega algo que se ve y se usa, no solo código por debajo.

| Fase | Entregable |
| --- | --- |
| 1 (hecha) | Proyecto CMake + Qt Quick; iconos; compilación de Windows y `.dmg` universal en CI, con prueba de humo |
| 2 (hecha) | Motor en C++ (fechas, festivos, plan, racha, ritmo) con Catch2 y paridad con la web; bienvenida en inglés con el perfil en SQLite; pantalla Hoy |
| 3 (hecha) | El plan diario de principio a fin: lecciones, repaso espaciado, los ocho entrenadores, controles de fase y simulacro B1, con el progreso en SQLite, comprobación de respuestas y voz. Desde la 0.3.1, voz neuronal propia y símbolos leídos como palabras. Desde la 0.3.2, el coach y la reflexión cada 3 sesiones. Desde la 0.3.3, teclado completo en pantalla. Desde la 0.3.4, voz Supertonic 2 con una voz por personaje. Desde la 0.3.5, voz más pausada, ronda de *Second try* al final y racha con icono o emoji. Desde la 0.3.6, ajustes con modo día y noche, la voz y su velocidad a elegir, y una voz menos robótica |
| **4 (actual)** | Pantallas para explorar (0.4.0): centro de práctica, palabras con buscador, inmersión con su registro, progreso con logros y señales de B1, y copia de seguridad compatible con la web. Los ajustes llegaron antes, en la 0.3.6. En la 0.4.5, voces arregladas: todas al mismo ritmo y volumen, pausas entre frases, una semilla por voz y Julián como narrador |
| 5 | Reconocimiento de voz en el propio equipo (whisper.cpp) y conversación con llama.cpp, que usa tu porqué como contexto |
