<!-- covers: CHANGELOG.md, docs/BRANCHING.md -->
# Committing

How a change is described: the shape of a commit message, which type to pick and what to keep out. [Branching and releases](BRANCHING.md) covers where a change lives.

## The format

```text
type(scope): what the change does

Optional body: why, when the code does not show it.
```

- **Subject:** imperative ("add", "fix", not "added"), at most 72 characters, no trailing period.
- **Scope:** the area touched, in parentheses: `vulkan`, `cmake`, `devcontainer`, `docs`. Leave it out when the change spans the whole project.
- **Body:** only for the reason. The diff already shows what changed.
- **Breaking change:** put `!` before the colon, as in `refactor(api)!: adopt Unreal type prefixes`. While the major version is 0, a breaking change bumps the minor version.

## Types

| Type | Use it for | Example from this repo |
|---|---|---|
| `feat` | new behavior a user of Nodens can see | `feat(vulkan): default graphics API to Vulkan` |
| `fix` | behavior that was wrong | `fix(cmake): warn when the Vulkan SDK is not found` |
| `refactor` | restructuring code with no change in behavior | `refactor(vulkan): split context module implementation` |
| `docs` | documentation only | `docs(branching): cite the verified Orosz pages and the devcontainer folder` |
| `build` | the build system, dependencies, packaging and the development environment | `build(devcontainer): one config per GPU type` |
| `ci` | the CI workflows | `ci: trust the workspace for git in the release job` |
| `style` | formatting only, no meaning change | `style: Enforce a single consecutive empty line` |
| `chore` | upkeep that fits nowhere above | `chore: ignore CMake system information files` |

When two types fit, pick the one a reader of the changelog would look under. A change that fixes behavior and refactors around it is a `fix`. The type does not change the route: every change goes through a pull request ([Branching and releases](BRANCHING.md)).

## One commit, one thing

- A commit does one thing and its subject names it. If the subject needs "and", it is two commits.
- Stage explicit paths (`git add path/to/file`), not `git add -A`, and read `git diff --staged` first.
- Unrelated changes go in separate commits, and a pull request holds one change.
- Every commit on `main` builds.

## Pull request titles

Pull requests are squash-merged, so the **PR title becomes the commit on `main`**. Write it in the same format as a commit subject, and check it before merging: GitHub appends the PR number, as in `fix(cmake): warn when the Vulkan SDK is not found (#40)`.

## The changelog

[`CHANGELOG.md`](../CHANGELOG.md) is written by hand in the Keep a Changelog sections (`Added`, `Changed`, `Fixed`, ...). A `feat`, `fix` or breaking change adds a line under `Unreleased` in the same pull request. `docs`, `build`, `ci`, `style`, `chore` and most `refactor` commits need none.
