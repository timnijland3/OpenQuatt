#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 3 && $# -ne 4 ]] || [[ $# -eq 4 && "$4" != "--include-legacy-eol-manifests" ]]; then
  echo "Usage: $0 <version> <base-url> <release-url> [--include-legacy-eol-manifests]" >&2
  exit 64
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec python3 "${ROOT_DIR}/scripts/build_targets.py" prepare-release-assets "$@"
