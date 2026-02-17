#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE' >&2
Usage: fixed_energy_union_legacy_alignment.sh <legacy_cparty_bin> <fixed_energy_tool> <param_file> <seed> <count> <lengths>

Compares legacy CParty MFE energy against fixed-energy union API using random (seq, G) inputs.
lengths: comma-separated list of lengths (e.g. 30,50,80)
USAGE
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

if [[ $# -ne 6 ]]; then
  usage
  exit 2
fi

legacy_bin="$1"
tool_bin="$2"
param_file="$3"
seed="$4"
count="$5"
lengths_csv="$6"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PARSER="$ROOT_DIR/test/tools/parse_cparty_stdout.sh"
GENERATOR="$ROOT_DIR/test/tools/gen_random_seq_struct.py"

abs_tol=1e-7
rel_tol=1e-7

if [[ ! -x "$legacy_bin" ]]; then
  echo "error: legacy bin not executable: $legacy_bin" >&2
  exit 2
fi
if [[ ! -x "$tool_bin" ]]; then
  echo "error: fixed-energy tool not executable: $tool_bin" >&2
  exit 2
fi
if [[ ! -f "$param_file" ]]; then
  echo "error: param file not found: $param_file" >&2
  exit 2
fi
if [[ ! -x "$PARSER" ]]; then
  echo "error: missing parser script: $PARSER" >&2
  exit 2
fi
if [[ ! -f "$GENERATOR" ]]; then
  echo "error: missing generator script: $GENERATOR" >&2
  exit 2
fi

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

abs_file="$tmpdir/abs_diffs.txt"
rel_file="$tmpdir/rel_diffs.txt"
touch "$abs_file" "$rel_file"

alignment_compared=0
alignment_mismatched=0
skipped=0
finite_count=0

IFS=',' read -r -a lengths <<<"$lengths_csv"
for length in "${lengths[@]}"; do
  if [[ -z "$length" ]]; then
    continue
  fi
  case_id_prefix="rand${length}"
  cases_file="$tmpdir/cases_${length}.tsv"
  if ! python3 "$GENERATOR" --seed "$seed" --count "$count" --length "$length" --turn 3 --prefix "$case_id_prefix" \
      > "$cases_file"; then
    echo "error: failed to generate cases for length $length" >&2
    exit 1
  fi

  while IFS=$'\t' read -r case_id seq g; do
    if [[ -z "$seq" || -z "$g" ]]; then
      skipped=$((skipped + 1))
      continue
    fi

    stdout_file="$tmpdir/legacy_${case_id}.txt"
    if ! "$legacy_bin" -d2 -P "$param_file" -r "$g" "$seq" > "$stdout_file" 2>/dev/null; then
      skipped=$((skipped + 1))
      continue
    fi

    if ! parsed=$("$PARSER" "$stdout_file" 2>/dev/null); then
      skipped=$((skipped + 1))
      continue
    fi

    cli_seq=$(printf '%s' "$parsed" | awk -F $'\t' '{print $1}')
    cli_restricted=$(printf '%s' "$parsed" | awk -F $'\t' '{print $2}')
    cli_mfe_structure=$(printf '%s' "$parsed" | awk -F $'\t' '{print $3}')
    cli_mfe_energy=$(printf '%s' "$parsed" | awk -F $'\t' '{print $4}')

    if [[ "$cli_seq" != "$seq" || "$cli_restricted" != "$g" ]]; then
      alignment_mismatched=$((alignment_mismatched + 1))
      continue
    fi

    invalid_chars=$(printf '%s' "$cli_mfe_structure" | tr -d '.()[]')
    if [[ -n "$invalid_chars" ]]; then
      skipped=$((skipped + 1))
      continue
    fi

    g_from_mfe=$(printf '%s' "$cli_mfe_structure" | sed -e 's/\[/./g' -e 's/\]/./g')
    gprime_from_mfe=$(printf '%s' "$cli_mfe_structure" | sed -e 's/(/./g' -e 's/)/./g')

    if ! api_energy=$("$tool_bin" "$seq" "$g_from_mfe" "$gprime_from_mfe" 2>/dev/null); then
      skipped=$((skipped + 1))
      continue
    fi

    if ! python3 - "$cli_mfe_energy" "$api_energy" <<'PY'; then
import math,sys
a=float(sys.argv[1]); b=float(sys.argv[2])
sys.exit(0 if math.isfinite(a) and math.isfinite(b) else 1)
PY
      skipped=$((skipped + 1))
      continue
    else
      finite_count=$((finite_count + 1))
    fi

    abs_diff=$(python3 - <<'PY' "$cli_mfe_energy" "$api_energy"
import sys,math
a=float(sys.argv[1]); b=float(sys.argv[2])
print(math.fabs(a-b))
PY
)
    scale=$(python3 - <<'PY' "$cli_mfe_energy" "$api_energy"
import sys,math
a=abs(float(sys.argv[1])); b=abs(float(sys.argv[2]))
print(max(a,b))
PY
)
    rel_diff=$(python3 - <<'PY' "$abs_diff" "$scale"
import sys
d=float(sys.argv[1]); s=float(sys.argv[2])
print(0.0 if s == 0.0 else d / s)
PY
)

    printf '%s\n' "$abs_diff" >> "$abs_file"
    printf '%s\n' "$rel_diff" >> "$rel_file"

    alignment_compared=$((alignment_compared + 1))

    if ! python3 - "$abs_diff" "$scale" "$abs_tol" "$rel_tol" <<'PY'; then
import sys
d=float(sys.argv[1]); s=float(sys.argv[2])
abs_tol=float(sys.argv[3]); rel_tol=float(sys.argv[4])
ok = (d <= abs_tol) or (s > 0.0 and d <= rel_tol * s)
sys.exit(0 if ok else 1)
PY
      alignment_mismatched=$((alignment_mismatched + 1))
    fi
  done < <(tail -n +2 "$cases_file")
done

median_abs=$(python3 - <<'PY' "$abs_file"
import sys,statistics
path=sys.argv[1]
vals=[float(line.strip()) for line in open(path) if line.strip()]
print(statistics.median(vals) if vals else 0.0)
PY
)
median_rel=$(python3 - <<'PY' "$rel_file"
import sys,statistics
path=sys.argv[1]
vals=[float(line.strip()) for line in open(path) if line.strip()]
print(statistics.median(vals) if vals else 0.0)
PY
)
top3_abs=$(python3 - <<'PY' "$abs_file"
import sys
path=sys.argv[1]
vals=[float(line.strip()) for line in open(path) if line.strip()]
vals.sort(reverse=True)
print(",".join(str(v) for v in vals[:3]) if vals else "0")
PY
)
top3_rel=$(python3 - <<'PY' "$rel_file"
import sys
path=sys.argv[1]
vals=[float(line.strip()) for line in open(path) if line.strip()]
vals.sort(reverse=True)
print(",".join(str(v) for v in vals[:3]) if vals else "0")
PY
)

finite_rate=0.0
if [[ "$alignment_compared" -gt 0 ]]; then
  finite_rate=$(python3 - <<'PY' "$finite_count" "$alignment_compared"
import sys
finite=int(sys.argv[1]); total=int(sys.argv[2])
print(100.0 * finite / total)
PY
)
fi

echo "alignment_compared=$alignment_compared"
echo "alignment_mismatched=$alignment_mismatched"
echo "skipped=$skipped"
echo "finite_rate=${finite_rate}%"
echo "abs_top3_max=$top3_abs"
echo "rel_top3_max=$top3_rel"
echo "abs_median=$median_abs"
echo "rel_median=$median_rel"

if [[ "$alignment_compared" -eq 0 ]]; then
  echo "alignment gate failed: alignment_compared=0" >&2
  exit 1
fi

if [[ "$alignment_mismatched" -ne 0 ]]; then
  echo "alignment gate failed: alignment_mismatched=$alignment_mismatched (expected 0)" >&2
  exit 1
fi
