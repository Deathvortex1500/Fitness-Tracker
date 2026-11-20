# Project instructions

- After implementing and verifying user-facing changes, build the Linux deliverable as an AppImage with `./scripts/build-appimage.sh`.
- Put release artifacts in `dist/` and report the resulting AppImage path in the handoff.
- The current application version is v0.86. For each future app update, increment the version by 0.01 everywhere it appears before building.
- If the requested increment would move the version past the `.09` boundary, stop and ask the user how they want the next version represented.
