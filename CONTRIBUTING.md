# Contributing

Thanks for contributing. This repository follows one workflow: issues first,
one branch per topic, conventional commits, pull requests only, `main`
always green.

## Setup

<!-- setup: replace this section with the actual toolchain commands -->

```sh
# install toolchain (mise/asdf or package manager of the project)
# install dependencies
# run the test suite once to confirm a green baseline
```

## Issues

- Open issues through the templates: **Bug report** or **Feature request**
  (search for duplicates first).
- Bug reports should include reproduction steps and the versions tested.
- **Security problems never go to public issues.** Use GitHub private
  vulnerability reporting (see `SECURITY.md`).
- Inactive issues may be marked stale after 23 days and closed 7 days later
  without updates.

## Branching

- One branch per topic, cut from `main`:
  `feat/<scope>`, `fix/<scope>`, `chore/<scope>`, `docs/<scope>`, `test/<scope>`.
- `main` stays green at all times.
- Keep branch scope small: one topic, one review, one merge.

## Commits

Conventional Commits, atomic, English:

```
feat(engine): emit rule provenance in result
fix(parser): reject nested quotes in string atoms
test(corpus): add regression case for stack6
docs(readme): clarify dry-run mode
chore(ci): bump actions group
```

- Types: `feat`, `fix`, `test`, `docs`, `chore`, `refactor`, `perf`.
- Subject <= 50 characters, imperative mood; the body explains *why*.
- One commit = one change **plus its test**.
- `CHANGELOG.md` `[Unreleased]` is updated in the same commit for `feat`/`fix`.

## Pull requests

`main` is protected: every change lands through a pull request.

1. Branch off `main`, implement, keep the suite green.
2. Open the PR: Conventional Commit title, `Fixes #N` when an issue exists,
   complete the template checklist honestly.
3. CI must be green (tests, lint, build).
4. Merge with a merge commit, delete the branch.

Review expectations scale with contributors: while the maintainer is the
single author, own PRs are self-merged; with a second contributor, reviews
become required.

## Releases

1. Move `CHANGELOG.md` `[Unreleased]` to a dated section.
2. Bump the project version.
3. Tag `vX.Y.Z` on `main` **after** the merge, on the exact merge commit,
   one tag at a time. Release artifacts publish from CI on the tag.

## Quality bar

- **TDD, no exceptions**: the failing test first, then the minimal change.
  Bug fixes ship a regression test that fails on `main` before the fix.
- Lint clean, no offenses.
- Docs updated when user-facing behavior changes.