# Fitness v0.87

A private, offline-first desktop fitness tracker built with C++20, Qt 6 Widgets,
and SQLite. It tracks daily nutrition, hydration, weight, workouts, reusable
recipes, goals, and week/month trends.

## Build and run

```bash
qmake6 FitnessTracker.pro
make -j"$(nproc)"
./Fitness
```

Data is stored locally in Qt's application-data directory as `fitness.db`.
On Linux this is normally `~/.local/share/Fitness/Fitness/`.

## Create an AppImage

Download `linuxdeploy` into `tools/` (the included script also supports a
system installation), then run:

```bash
./scripts/build-appimage.sh
```

The script produces `dist/Fitness-v0.87-x86_64.AppImage`. The same Qt source
can be compiled for Windows with Qt's Windows toolchain. A future web client can
reuse the data model and field names behind a small API.

## Project map

- `src/datastore.*` — SQLite schema and all persistence operations.
- `src/mainwindow.*` — the navigation, pages, forms, and application behavior.
- `src/chartwidget.*` — dependency-free trend graph rendering.
- `packaging/` — Linux desktop metadata and icon.
- `web/` — mobile-first offline PWA, ready for GitHub Pages.

The code is commented around architectural decisions and non-obvious behavior;
function and variable names carry the detail elsewhere to keep it readable.
