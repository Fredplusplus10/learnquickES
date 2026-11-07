# Asset inventory and usage

## Visual identity

`assets/brand/` contains the editable LearnQuick mark: symbol, wordmark, monochrome alternative and macOS-style application icon. SVG is the source format; raster exports are provided only for contexts that do not accept vectors.

## Repository presentation

- `assets/preview/github-social-preview.png`: 1280 × 640; optimized for repository sharing.
- `assets/preview/readme-header.png`: 1200 × 270; wide lightweight README header.
- `app/assets/illustrations/empty-*.svg` (the same files in `desktop/assets/illustrations/`): empty-state illustrations, 240 × 180 with a transparent background, so one file works on light and dark cards; only SVG Tiny features (no filters, masks or text), which Qt draws.
- `assets/screenshots/windows-*.png` (and `macos-*.png`): real screenshots of the desktop app with `--screenshot --demo --today 2026-11-02 --open <route> --theme light`, at its default window size. Windows ones use the native platform, not `offscreen`, which picks the wrong font on Windows.
- `assets/screenshots/app-dashboard.webp`, `app-lesson.webp`, `app-practice.webp`, `app-progress.webp`: real screenshots of the application with its demo learner (1440 × 900).

The screenshots show the real interface with demo data: none contains measured learning data from a real user.

## Source-controlled application resources

The `app/assets/` tree contains the icons, illustrations, logos and app-icon sizes used by the application. The `assets/` directory at repository root is for GitHub documentation and public-facing project identity.

## Practical use

- Prefer SVG for UI artwork and logo placement.
- Export PNG for repository social previews; GitHub recommends 1280 × 640 at less than 1 MB.
- Keep decorative illustrations independent from real UI controls.
- Supply alternative text for informational images and empty alternative text for decorative images.
- Avoid embedding proprietary fonts, CDN-hosted dependencies or third-party icon kits without attribution and compatible license terms.

For publication outside the project, review the final logo, naming and trademark situation separately; the included concepts do not establish trademark registration.
