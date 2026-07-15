# Repository Hygiene

Last updated: 2026-04-25

## Goals

- Keep commits small, reviewable, and reversible.
- Separate source changes from generated files, local saves, logs, and dependency caches.
- Preserve the original game data and rules while making modernization work auditable.

## Commit Categories

Use one logical commit per category:

- `docs`: handoff, roadmap, audit, architecture notes, licensing notes.
- `build`: CMake, dependency discovery, CI, packaging scripts.
- `test`: Unity tests, test runners, test utilities.
- `fix`: targeted bug or security fixes.
- `feat`: new player-visible functionality.
- `refactor`: behavior-preserving cleanup.
- `chore`: ignore files, repository maintenance, tooling.

## Generated Or Local Files

These should normally stay untracked:

- `build/`, `build-*/`, CMake cache files, Visual Studio projects, object files, executables.
- `lib/logs/` and `*.log`.
- `lib/save/` and player save files created during manual testing.
- IDE caches such as `.vs/`, `.vscode/`, `.cache/`.

If a generated artifact must be committed, explain why in the PR and keep it isolated from source edits.

## Third-Party Code

- `third_party/unity/` is vendored test infrastructure under MIT. Keep its license file with the source.
- Do not commit Steamworks SDK files unless the license explicitly allows it. Prefer local SDK discovery and documentation.
- For future asset packs, create `third_party/assets/<pack-name>/` only after license verification.

## Historical Agent Files

`agent-os/` is historical unless a future cleanup PR removes it or migrates specific content. Do not use it as the active source of truth.

## Review Checklist

Before staging a PR:

1. Run `git status --short` and classify every changed path.
2. Exclude build outputs, logs, saves, caches, and unrelated local files.
3. Confirm docs match code and command output.
4. Run the narrowest relevant tests, then `ctest -C Debug --output-on-failure` when the build is available.
5. Update `HANDOFF.md` for any significant state change.
