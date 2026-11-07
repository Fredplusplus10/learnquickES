# Design system

## Visual direction

LearnQuick follows a desktop-oriented design language: restrained chrome, explicit information hierarchy, light surfaces, warm orange interaction states, consistent iconography, and task-focused visual density.

The concept is compatible with macOS visual conventions without copying system UI assets. Controls and branding are custom-designed vector resources.

## Design tokens

| Token | Hex | Intent |
| --- | --- | --- |
| `surface.canvas` | `#FAF7F4` | Main application background |
| `surface.card` | `#FFFDFB` | Card surfaces |
| `surface.sidebar` | `#F6F0ED` | Navigation area |
| `text.primary` | `#36231E` | Headings and high-contrast copy |
| `text.secondary` | `#796F6A` | Supporting descriptions |
| `brand.primary` | `#ED712F` | Primary actions and emphasis |
| `brand.active` | `#D8541B` | Active / high-emphasis elements |
| `brand.subtle` | `#FFF0E5` | Selected and highlighted backgrounds |
| `border.default` | `#E9DED7` | Card edges and separators |
| `status.success` | `#3D8B59` | Confirmed correct answers |

Colors in this table come from the original design and are used unchanged by the app (`app/css/kit.css`). A production accessibility audit may require adjustments to secondary text and small controls.

## Typography

Prefer **system-provided macOS fonts** in the native Qt implementation, with appropriate fallbacks. Do not redistribute proprietary or downloaded font files in the repository. Typical scales: title 30–43 px in the web concept, section headings 16–20 px, body 13–15 px, metadata 11–12 px. Production sizes should be validated in physical points and under system scaling.

## Component inventory

| Component | Responsibility |
| --- | --- |
| `ApplicationShell` | Window structure, toolbar, sidebar and navigation history |
| `ProgressOverview` | Current study day, progress ring, remaining weekdays |
| `DailyStudyList` | Task breakdown with duration estimates |
| `VocabularyTable` | Spanish terms, English equivalents, audio triggers |
| `LessonPlayer` | Phrase presentation, translation and listening control |
| `AnswerOptions` | Accessible selection and feedback |
| `RoadmapTimeline` | Milestone states and study phases |
| `AnalyticsChart` | Historical metrics with explicit units |
| `AchievementGrid` | Earned and locked achievement states |

## Layout conventions

- Use a stable left navigation area and an independently scrollable content region.
- Limit simultaneous primary calls to action; avoid multiple visually equivalent primary buttons.
- Keep a minimum 44 x 44 logical-point interactive target where practical.
- Apply a consistent spacing scale (4, 8, 12, 16, 24, 32) across components.
- Use sufficient padding for Spanish punctuation and languages with longer words.
- Respect reduced-motion and high-contrast preferences.

## Branding

The **open book** communicates structured learning. The **lightning bolt** indicates rapid, focused practice. The orange gradient is reserved for the brand mark and primary interactive accents. Maintain adequate clear space around the wordmark and avoid stretching, rotating, applying glow effects, or recoloring the mark arbitrarily.

Original assets are stored in `assets/brand/`; the icons and illustrations used by the app are in `app/assets/`. See `ASSET_GUIDELINES.md`.
