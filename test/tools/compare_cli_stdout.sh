#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE' >&2
Usage: compare_cli_stdout.sh <legacy_cparty_bin> <current_cparty_bin> <seed> <count> <lengths>

Compares CLI stdout (seq/restricted/MFE/energy) between legacy and current binaries.
Inputs are generated from pk_free templates in multi.secstruct, filtered by length.
Lengths is a comma-separated list (e.g. "30,50,80").
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
SELECTOR="$ROOT_DIR/test/tools/select_legacy_stable_cases.sh"
COMPARE_TSV="$ROOT_DIR/test/tools/compare_cli_from_tsv.sh"

if [[ ! -x "$legacy_bin" ]]; then
  echo "error: legacy binary not executable: $legacy_bin" >&2
  exit 1
fi
if [[ ! -x "$current_bin" ]]; then
  echo "error: current binary not executable: $current_bin" >&2
  exit 1
fi
if [[ ! -x "$SELECTOR" ]]; then
  echo "error: missing selector script: $SELECTOR" >&2
  exit 1
fi
if [[ ! -x "$COMPARE_TSV" ]]; then
  echo "error: missing TSV compare script: $COMPARE_TSV" >&2
  exit 1
fi

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

cases_tsv="$tmpdir/cases.tsv"

IFS=',' read -r -a lengths <<< "$lengths_csv"
if [[ ${#lengths[@]} -eq 0 ]]; then
  echo "error: no lengths provided" >&2
  exit 1
fi
for L in "${lengths[@]}"; do
  if ! [[ "$L" =~ ^[0-9]+$ ]]; then
    echo "error: invalid length: $L" >&2
    exit 1
  fi
done

"$SELECTOR" "$legacy_bin" "$seed" "$count" "$lengths_csv" "$cases_tsv"
"$COMPARE_TSV" "$legacy_bin" "$current_bin" "$cases_tsv"
