#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE' >&2
Usage: compare_cli_from_tsv.sh <legacy_cparty_bin> <current_cparty_bin> <cases_tsv>

Compares CLI stdout (seq/restricted/MFE/energy) between legacy and current binaries
for each row in cases_tsv (header: case_id seq G).
USAGE
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

if [[ $# -ne 3 ]]; then
  usage
  exit 2
fi

legacy_bin="$1"
current_bin="$2"
cases_tsv="$3"

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
if [[ ! -f "$cases_tsv" ]]; then
  echo "error: cases TSV not found: $cases_tsv" >&2
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

total=0
matched=0
skipped=0
failed=0

while IFS=$'\t' read -r case_id seq g; do
  if [[ "$case_id" == "case_id" ]]; then
    continue
  fi
  total=$((total + 1))

  if ! "$legacy_bin" -d2 -P "$PARAM_FILE" -r "$g" "$seq" > "$stdout_legacy" 2>/dev/null; then
    skipped=$((skipped + 1))
    continue
  fi
  if ! "$current_bin" -d2 -P "$PARAM_FILE" -r "$g" "$seq" > "$stdout_current" 2>/dev/null; then
    skipped=$((skipped + 1))
    continue
  fi

  if ! legacy_parsed=$("$PARSER" "$stdout_legacy" 2>/dev/null); then
    skipped=$((skipped + 1))
    continue
  fi
  if ! current_parsed=$("$PARSER" "$stdout_current" 2>/dev/null); then
    skipped=$((skipped + 1))
    continue
  fi

  if [[ "$legacy_parsed" != "$current_parsed" ]]; then
    failed=$((failed + 1))
  else
    matched=$((matched + 1))
  fi
done < "$cases_tsv"

echo "compare_total=$total"
echo "compare_matched=$matched"
echo "compare_failed=$failed"
echo "compare_skipped=$skipped"
compare_min="${COMPARE_MIN_MATCHED:-100}"
compared=$((matched + failed))
echo "compare_compared=$compared"
echo "compare_min=$compare_min"

if (( total < 1 )); then
  echo "compare gate failed: no cases executed" >&2
  exit 1
fi
if (( compared < compare_min )); then
  echo "compare gate failed: compared=$compared (<$compare_min)" >&2
  exit 1
fi
if (( failed != 0 )); then
  echo "compare gate failed: mismatched=$failed" >&2
  exit 1
fi

exit 0
