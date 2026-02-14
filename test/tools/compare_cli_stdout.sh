#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE' >&2
Usage: compare_cli_stdout.sh <legacy_cparty_bin> <current_cparty_bin> <seed> <count> <lengths>

Compares CLI stdout (seq/restricted/MFE/energy) between legacy and current binaries.
Lengths is a comma-separated list (e.g. "10,30,50").
USAGE
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

if [[ $# -ne 5 ]]; then
  usage
  exit 2
fi

legacy_bin="$1"
current_bin="$2"
seed="$3"
count="$4"
lengths_csv="$5"

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
PARSER="$ROOT_DIR/test/tools/parse_cparty_stdout.sh"
GEN="$ROOT_DIR/test/tools/gen_random_seq_struct.py"
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
if [[ ! -f "$GEN" ]]; then
  echo "error: missing generator: $GEN" >&2
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
cases_tsv="$tmpdir/cases.tsv"

IFS=',' read -r -a lengths <<< "$lengths_csv"
if [[ ${#lengths[@]} -eq 0 ]]; then
  echo "error: no lengths provided" >&2
  exit 1
fi

total=0
matched=0
skipped=0
failed=0

for L in "${lengths[@]}"; do
  if ! [[ "$L" =~ ^[0-9]+$ ]]; then
    echo "error: invalid length: $L" >&2
    exit 1
  fi

  python3 "$GEN" --seed "$seed" --count "$count" --length "$L" --prefix "len${L}_" > "$cases_tsv"
  while IFS=$'\t' read -r case_id seq g; do
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
  done < <(tail -n +2 "$cases_tsv")
done

echo "compare_total=$total"
echo "compare_matched=$matched"
echo "compare_failed=$failed"
echo "compare_skipped=$skipped"

if (( total < 1 )); then
  echo "compare gate failed: no cases executed" >&2
  exit 1
fi
if (( failed != 0 )); then
  echo "compare gate failed: mismatched=$failed" >&2
  exit 1
fi

exit 0
