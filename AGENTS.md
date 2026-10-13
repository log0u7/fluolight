# AGENTS.md

Guidance for coding agents working in this repository.

## Project

<!-- describe the project in 2-4 lines -->

## Commands

<!-- setup: replace with real commands -->

- Test: `# run the test suite (all green required)`
- Lint: `# run the linter (zero offenses required)`
- Build: `# run the build`

## Workflow

- `main` is protected: every change lands through a pull request
  (see `CONTRIBUTING.md` for the full flow).
- One topic per branch, cut from `main`: `feat|fix|chore|docs|test/<scope>`.
- One session = one worktree:
  `git worktree add ~/projets/wt/<repo>/<topic> -b <topic>`.
  The main clone stays on `main` and is never mutated by working sessions.
- TDD is mandatory: failing test first, minimal implementation, regression
  test for bug fixes before the fix.
- Conventional Commits, English only, atomic (one change + its test).
- `CHANGELOG.md` `[Unreleased]` updated in the same commit for feat/fix.
- Tags `vX.Y.Z` go on `main` only after the merge lands, on the exact merge
  commit, pushed one by one.
- Secrets never land here: scanning runs in pre-commit and CI.

## Non-negotiable rules

<!-- project-specific invariants, e.g. dependency policy, security posture -->

1. Validate inputs at trust boundaries; fail loudly with explicit errors.
2. No hardcoded secrets; least privilege everywhere.
3. Keep the attack surface minimal; deny by default.