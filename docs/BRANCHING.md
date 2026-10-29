<!-- covers: CMakeLists.txt, .devcontainer/, .github/workflows/ -->
# Branching and releases

Nodens keeps one trunk. Every change integrates into `main` through a short pull request, and a release is an annotated tag on `main` followed by a CI job that publishes it. A single shared main branch, rather than long-lived branches merged into a release line infrequently, is what trunk-based development means.[^orosz]

## Branches

| Branch | Purpose | Lifetime | Ends by |
|---|---|---|---|
| `main` | The trunk and the only published lineage. Every commit on it builds. | Permanent | Never |
| `feature/<slug>` | One change of any type, cut from `main`. | Hours to a few days | Squashed into `main` through a pull request |
| `hotfix/<slug>` | A fix for a tag that already shipped, cut from that tag. | Hours | Squashed into `main`, then a patch tag on `main` |
| `release/<version>` | A released line that must stay stable while `main` moves on. Keep one only while two versions must be supported. | One release cycle | Deleted after the last patch tag |

## Rules

1. `main` is kept green.[^orosz-health] A failing `main` is fixed before any other change, and a tag is created only on a commit whose CI run succeeded.
2. Every change goes through a pull request and lands as one squashed commit, whatever its type. Nothing is committed directly to `main`. Title conventions are in [Committing](COMMITTING.md).
3. Tags are annotated, named `vMAJOR.MINOR.PATCH`, and created on `main` only.
4. While the major version is 0, a breaking change bumps the minor version: 0.2.0 becomes 0.3.0, never 0.2.1.
5. The tag name and the CMake project version must agree, because the release job refuses to publish when they do not (the version is declared in `CMakeLists.txt`).

## Making a change

```bash
git switch main
git pull --ff-only
git switch -c feature/texture-uploads
# work, commit
git push -u origin feature/texture-uploads
gh pr create --base main --fill
```

Merge it with "Squash and merge" once CI is green, then delete the branch.

## Cutting a release

On a branch cut from `main`:

- give the version a dated section in `CHANGELOG.md`, moved out of `Unreleased`; the release job publishes that section as the release notes
- set `project(Nodens VERSION ...)` in `CMakeLists.txt` to the new version

Open a pull request titled `chore(release): prepare X.Y.Z` and squash-merge it. Then tag the merge commit, only once its CI run has succeeded:

```bash
git switch main
git pull --ff-only
gh run list --branch main --commit "$(git rev-parse HEAD)"
git tag -a vX.Y.Z -m "Nodens X.Y.Z"
git push origin vX.Y.Z
```

The tag push runs the release job. It builds the tag, checks the version against the project declaration, uploads a source archive and creates the GitHub Release, titled with the bare version and with the `CHANGELOG.md` section as its notes. The job fails when the section is missing.

## Fixing a shipped version

```bash
git switch -c hotfix/X.Y.Z v<previous>
# fix, commit
git push -u origin hotfix/X.Y.Z
gh pr create --base main --fill
```

Squash-merge it, then tag `main` as in a normal release. The fix lands on `main` first. When a `release/<version>` branch exists, cherry-pick the commit onto it and tag the patch there instead.

## What CI checks

| Event | Jobs | What it proves |
|---|---|---|
| Pull request into `main` | `format`, `docs`, `build` for Debug, Release and Profiling | the change compiles in all three configurations, keeps the Vulkan backend, and is formatted |
| Push to `main` | the same jobs | the trunk stays releasable after each merge |
| Tag `v*` | `build`, `release` | the tagged commit builds and the release is published from it |

The jobs run inside the project's own development image, so CI uses the same compiler and CMake as the dev container (the image is named in each config under `.devcontainer/`). They also assert that the Vulkan backend was compiled in, because `CMakeLists.txt` looks for Vulkan with `find_package(Vulkan QUIET)` and a missing SDK would otherwise reduce the build to OpenGL without anyone noticing.

## Common slips

- Tagging before the CI run on that commit has finished.
- A branch that lives for weeks is deferred integration, not a strategy. Either give it a version number and call it a release branch, or split it.
- Tagging from a hotfix branch instead of `main` leaves the fix out of the trunk.
- Editing `CHANGELOG.md` after the tag produces notes that describe a tree that no longer exists.
- Moving the version in `CMakeLists.txt` without tagging, or tagging without moving it, trips the release job on purpose.

## References

[^orosz]: Gergely Orosz, *The Software Engineer's Guidebook: Navigating Senior, Tech Lead, and Staff Engineer Positions at Tech Companies and Startups*, First edition (Amsterdam, Netherlands: The Pragmatic Engineer, 2023), p. 329.

[^orosz-health]: Orosz, *The Software Engineer's Guidebook*, p. 330.
