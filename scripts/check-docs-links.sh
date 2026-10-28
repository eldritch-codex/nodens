#!/usr/bin/env bash
# Fail on a relative Markdown link under docs/ or in the repository's own
# Markdown files that points at a path which is not in the tree. Run by the CI
# `docs` job, so a moved page breaks the pull request that moves it, not a
# reader's click.
#
# External URLs and pure anchors (#section) are not checked: they need the
# network, and a stale anchor is a scroll rather than a broken page.
#
# Usage: bash scripts/check-docs-links.sh
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."

status=0

while IFS= read -r file; do
  dir="$(dirname "${file}")"
  while IFS= read -r target; do
    case "${target}" in
      '' | '#'* | http://* | https://* | mailto:* | *://*) continue ;;
    esac
    # Drop the fragment, a link title, and the optional angle brackets.
    path="${target%%#*}"
    path="${path%% *}"
    path="${path#<}"
    path="${path%>}"
    [ -z "${path}" ] && continue
    case "${path}" in
      /*) resolved="${path}" ;;
      *) resolved="${dir}/${path}" ;;
    esac
    if [ ! -e "${resolved}" ]; then
      echo "${file}: links to missing ${target}"
      status=1
    fi
  done < <(awk '/^```/ { fence = !fence; next } !fence' "${file}" | grep -oE '\]\([^)]*\)' | sed -e 's/^](//' -e 's/)$//' || true)
done < <(git ls-files '*.md' | sort -u)

if [ "${status}" -ne 0 ]; then
  echo "broken relative links in the Markdown" >&2
  exit 1
fi

echo "docs links ok"
