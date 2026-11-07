# Guía de desarrollo (español)

> El README principal del repositorio está en inglés. Este documento es la versión en español para el mantenedor.

App local para un único alumno (estadounidense, con MacBook) que quiere llegar a un **español B1 funcional**. Funciona sin instalar nada, sin internet y sin cuenta: se abre con doble clic y guarda el progreso en el propio Mac.

- **Alumno:** lee [`docs/STUDENT-GUIDE.md`](STUDENT-GUIDE.md) (en inglés).
- **Por qué está hecha así:** [`docs/INVESTIGACION-Y-METODO.md`](INVESTIGACION-Y-METODO.md) (investigación, conclusiones y fuentes).

## Abrirla

1. Copiar la carpeta `LearnQuick` entera al Mac (por ejemplo en *Documentos*).
2. Doble clic en **`LearnQuick.html`**. Se abre en el navegador por defecto (Safari recomendado) y empieza la bienvenida.
3. Opcional: arrastrar `LearnQuick.html` al Dock para tenerla a mano.

Para ver la app con datos de ejemplo (alumno ficticio en el día 16 de 60), abrir `app/index.html?demo` o pulsar *Explore a demo* en la bienvenida. La demo usa su propio almacenamiento y no toca el progreso real.

## Qué incluye

| Área | Contenido |
|---|---|
| Plan | 30, 60 o 90 días de estudio (lunes a viernes por defecto; fines de semana, festivos federales de EE. UU. y días libres no cuentan). Recomendación según prueba de nivel de 15 preguntas. |
| Lecciones | 25 unidades en 5 fases (Fundamentos 80/20 → Conversaciones → Pasado → Planes y opiniones → B1 real). Cada una: 12 palabras con artículo, un *hack* de gramática, diálogo con audio, ejercicios y frases para decir en voz alta. 300 palabras y 230 ejercicios en total. |
| Racha | Tipo Duolingo: el día cuenta con 10 XP; hasta 2 protectores (se ganan cada 5 días de racha o estudiando en un día libre); los días no lectivos nunca la rompen. La meta del día va aparte de la racha. |
| Entrenadores | Verbos 80/20 (25 verbos, tiempos que se desbloquean con las lecciones), fui vs. iba (32 frases), cognados (16 reglas), falsos amigos (20), el/la (51 palabras y reglas), pronunciación y *shadowing*, «descríbelo en español», práctica mixta y repaso espaciado (Leitner). |
| Inmersión | Registro de minutos de escucha (objetivo diario según el plan), hitos por horas al estilo Dreaming Spanish, recursos A2–B1 (Dreaming Spanish, No Hay Tos, Español con Juan…), misiones «Goodbye English» para el iPhone y el Mac, intercambios en Tandem/HelloTalk, y la **isla lingüística**: una hora cronometrada en la que toda la app pasa a español. |
| Evaluación | Checkpoint al final de cada fase (15 preguntas, aprobado 70 %) y simulacro B1 final modelado sobre el DELE B1 (lectura, audio, gramática, escritura y 4 tareas orales cronometradas). |
| Progreso | Hoja de ruta, minutos por día, precisión por destreza, últimas 5 semanas de racha, señales de B1 y 16 logros. |
| Ajustes | Duración del plan, días de estudio, días libres, recordatorio para Calendario (.ics), acento y voz, velocidad, interfaz en inglés o español, vosotros, copia de seguridad y restauración. |

## Estructura

```
LearnQuick/
├── LearnQuick.html          ← lanzador (redirige a app/index.html)
├── app/
│   ├── index.html           ← carcasa del kit + scripts en orden
│   ├── css/kit.css          ← estilos del Design Kit (+ la bandera SVG del kit de repositorio)
│   ├── css/app.css          ← capa propia de la app
│   ├── assets/              ← iconos, ilustraciones y logos del kit
│   └── js/
│       ├── core/            ← lógica pura: util, calendar, srs, streak, verbs, progress, store
│       ├── content/         ← contenido: units.js (25 lecciones), practice.js (entrenadores, recursos, simulacro)
│       ├── ui/              ← components, speech (voz), exercises (motor de sesiones)
│       ├── views/           ← una vista por pantalla
│       └── app.js           ← rutas, eventos delegados, barra superior, búsqueda ⌘K, contador de tiempo
├── tests/                   ← 20 pruebas con node:test
├── desktop/                 ← app nativa en C++20 + Qt 6 (fase 3: el plan diario completo)
├── assets/                  ← identidad, vista previa de GitHub y capturas
├── content/lessons/         ← ejemplo de lección en JSON (formato futuro)
├── scripts/                 ← validación estática del repositorio (Python)
├── .github/                 ← CI, plantillas de issues y de pull requests
└── docs/
```

## Arquitectura (resumen)

- **Sin dependencias ni compilación.** HTML, CSS y JavaScript clásico (sin módulos ES ni `fetch`), porque los navegadores bloquean ambos al abrir un archivo local (`file://`). Cada script añade su parte al espacio global `window.LQ` y también se puede cargar en Node para las pruebas.
- **Rutas por hash** (`#/today`, `#/lesson/u05`, `#/practice/verbs`…). Cada vista devuelve HTML o `{ html, mount }`. Los clics se gestionan con atributos `data-go`, `data-action`, `data-speak`, etc.
- **Datos:** `localStorage` con la clave `learnquick.v1` (la demo usa `learnquick.demo.v1`). Se pide almacenamiento persistente al navegador y la app recuerda cada semana descargar una copia JSON. El formato está versionado (`migrate`).
- **Voz:** `speechSynthesis` con las voces instaladas en macOS (acento por defecto es-MX, configurable; prefiere voces «Enhanced/Premium»; 0,9×). Grabar y reconocer voz solo aparecen si el navegador lo permite; la app **nunca inventa una nota de pronunciación**.
- **Idioma de la interfaz:** inglés por defecto, español en Ajustes o automáticamente durante la isla lingüística (`LQ.L(en, es)`).

## Pruebas

Requiere Node 18 o superior y Python 3 (solo para desarrollar; el alumno no los necesita):

```bash
node --test tests/core.test.js tests/content.test.js
python3 scripts/validate_repository.py
```

GitHub Actions ejecuta lo mismo en cada push (`.github/workflows/verify.yml`).

Cubren calendario (festivos, horario de verano), reglas de racha y protectores, SRS, corrección de respuestas (acentos), las conjugaciones de los 25 verbos en todos los tiempos, una simulación completa del plan de 30 días, la integridad de las 25 unidades y que todos los ejercicios tienen solución. Resultado actual: **20/20**.

Además se recorrió la app en Chrome sin interfaz (demo completa y primer uso con prueba de nivel, recarga y cambio de plan): **0 errores de consola**.

## Pendiente de comprobar en el Mac del alumno

Se desarrolló y probó en Windows con Chrome; estas cosas solo se pueden confirmar en su Mac:

1. **Safari y `file://`:** que el progreso se guarde tras cerrar y reabrir. Si Safari lo borrase, la copia de seguridad semanal lo recupera; la solución definitiva es la app nativa (`desktop/`), que guardará el progreso en disco.
2. **Voces:** instalar una voz española mejorada (Ajustes del Sistema → Accesibilidad → Contenido leído → Voz del sistema → Gestionar voces → Español). Probar en *Settings → Voice → Play a sample*.
3. **Micrófono:** «Record me» y «Check words» dependen del navegador. En Chrome no funcionan desde `file://`; en Safari deberían. El reconocimiento de Safari envía el audio a Apple. Si no hay micrófono, «Listen & repeat» cubre la práctica oral.
4. **Emoji de bandera:** en Windows se ve «ES»; en macOS se ve la bandera.

## App nativa (C++20 + Qt 6)

Se construye en [`desktop/`](../desktop/README.md), con instalación del entorno, compilación y fases. La versión web sigue siendo la que se usa mientras tanto, y es la especificación de la nativa.
