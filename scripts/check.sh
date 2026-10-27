#!/usr/bin/env bash
# Local check before pushing a direct commit that touches C++ or the build.
# Run it in the dev container. It does what the CI `format` job and a Debug
# `build` job do, in a separate build directory (build/check) so it never
# changes the tree you are working in:
#
#   1. clang-format on the C++ files changed from the base (default origin/main),
#      including uncommitted and new files
#   2. configure and build Debug with Clang and Ninja
#
# Usage: bash scripts/check.sh [base-ref]
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."
base="${1:-origin/main}"

echo "== clang-format, files changed from ${base}"
files="$(
  {
    git diff --name-only --diff-filter=ACMR "${base}...HEAD"
    git diff --name-only --diff-filter=ACMR HEAD
    git ls-files --others --exclude-standard
  } | sort -u | grep -E '\.(cpp|cppm|h|hpp|hxx)$' | grep -v '^vendor/' || true
)"
if [ -n "${files}" ]; then
  echo "${files}"
  echo "${files}" | xargs clang-format --dry-run -Werror
else
  echo "no C++ files changed"
fi

echo "== configure and build Debug in build/check"
cmake -S . -B build/check -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/check --parallel

echo "check passed"
