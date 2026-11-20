# Fitness PWA v0.83

This is the separate, mobile-first web edition of Fitness Tracker. It stores all
profile, nutrition, workout, pantry, recipe, photo, and settings data locally in
the browser using IndexedDB. No personal data is sent to GitHub Pages.

## Publish with GitHub Pages

1. Push the repository to GitHub with `main` as its default branch.
2. Open **Settings → Pages** in the GitHub repository.
3. Under **Build and deployment**, select **GitHub Actions** as the source.
4. Run the **Deploy Fitness PWA to GitHub Pages** workflow, or push a change
   under `web/`.
5. Open the resulting URL in Safari on iPhone, use the Share button, and choose
   **Add to Home Screen**.

Use **Settings → Download JSON backup** regularly. Safari website-data removal
also removes this app's local database, so the downloaded backup is the recovery
path.
