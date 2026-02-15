#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE' >&2
Usage: compare_cli_single.sh <legacy_cparty_bin> <current_cparty_bin> <seq> <structure>

Runs both binaries with the same (seq, structure) and compares parsed stdout
fields: seq, restricted, mfe_structure, mfe_energy. Exits non-zero on mismatch.
USAGE
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

if [[ $# -ne 4 ]]; then
  usage
  exit 2
fi

legacy_bin="$1"
current_bin="$2"
seq="$3"
structure="$4"

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
PARSER="$ROOT_DIR/test/tools/parse_cparty_stdout.sh"
PARAM_FILE="$ROOT_DIR/params/rna_DirksPierce09.par"

if [[ ! -x "$legacy_bin" ]]; then
  echo "error: legacy binary not executable: $legacy_bin" >&2
  exit 1
fi
if [[ ! -x "$current_bin" ]]; then
  echo "error: current binary not executable: $current_bin" >&2
  exit 1
fi
if [[ ! -x "$PARSER" ]]; then
  echo "error: missing parser script: $PARSER" >&2
  exit 1
fi
if [[ ! -f "$PARAM_FILE" ]]; then
  echo "error: missing parameter file: $PARAM_FILE" >&2
  exit 1
fi

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

stdout_legacy="$tmpdir/legacy.txt"
stdout_current="$tmpdir/current.txt"

if ! "$legacy_bin" -d2 -P "$PARAM_FILE" -r "$structure" "$seq" > "$stdout_legacy" 2>/dev/null; then
  echo "error: legacy binary failed for seq=$seq structure=$structure" >&2
  exit 1
fi
if ! "$current_bin" -d2 -P "$PARAM_FILE" -r "$structure" "$seq" > "$stdout_current" 2>/dev/null; then
  echo "error: current binary failed for seq=$seq structure=$structure" >&2
  exit 1
fi

if ! legacy_parsed=$("$PARSER" "$stdout_legacy" 2>/dev/null); then
  echo "parse error: legacy stdout" >&2
  sed -n '1,20p' "$stdout_legacy" >&2
  exit 1
fi
if ! current_parsed=$("$PARSER" "$stdout_current" 2>/dev/null); then
  echo "parse error: current stdout" >&2
  sed -n '1,20p' "$stdout_current" >&2
  exit 1
fi

if [[ "$legacy_parsed" != "$current_parsed" ]]; then
  echo "mismatch detected" >&2
  echo "legacy:  $legacy_parsed" >&2
  echo "current: $current_parsed" >&2
  exit 1
fi

echo "match: $legacy_parsed"
