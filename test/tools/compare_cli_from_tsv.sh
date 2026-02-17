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
mfe_failed=0
pf_failed=0
pf_structure_failed=0
pf_energy_failed=0
legacy_pf_inf=0
current_pf_inf=0
legacy_pf_nan=0
current_pf_nan=0
legacy_pf_has_square=0
current_pf_has_square=0
legacy_pf_has_round=0
current_pf_has_round=0
legacy_pf_finite=0
current_pf_finite=0
sample_limit=5
sampled=0

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

  legacy_seq=$(printf '%s' "$legacy_parsed" | awk -F $'\t' '{print $1}')
  legacy_restricted=$(printf '%s' "$legacy_parsed" | awk -F $'\t' '{print $2}')
  legacy_mfe_structure=$(printf '%s' "$legacy_parsed" | awk -F $'\t' '{print $3}')
  legacy_mfe_energy=$(printf '%s' "$legacy_parsed" | awk -F $'\t' '{print $4}')
  legacy_pf_structure=$(printf '%s' "$legacy_parsed" | awk -F $'\t' '{print $5}')
  legacy_pf_energy=$(printf '%s' "$legacy_parsed" | awk -F $'\t' '{print $6}')

  current_seq=$(printf '%s' "$current_parsed" | awk -F $'\t' '{print $1}')
  current_restricted=$(printf '%s' "$current_parsed" | awk -F $'\t' '{print $2}')
  current_mfe_structure=$(printf '%s' "$current_parsed" | awk -F $'\t' '{print $3}')
  current_mfe_energy=$(printf '%s' "$current_parsed" | awk -F $'\t' '{print $4}')
  current_pf_structure=$(printf '%s' "$current_parsed" | awk -F $'\t' '{print $5}')
  current_pf_energy=$(printf '%s' "$current_parsed" | awk -F $'\t' '{print $6}')

  mfe_ok=1
  pf_ok=1
  if [[ "$legacy_seq" != "$current_seq" || "$legacy_restricted" != "$current_restricted" ]]; then
    mfe_ok=0
    pf_ok=0
  else
    if [[ "$legacy_mfe_structure" != "$current_mfe_structure" || "$legacy_mfe_energy" != "$current_mfe_energy" ]]; then
      mfe_ok=0
    fi
    if [[ "$legacy_pf_structure" != "$current_pf_structure" || "$legacy_pf_energy" != "$current_pf_energy" ]]; then
      pf_ok=0
    fi
  fi

  if [[ "$legacy_pf_structure" == *"["* || "$legacy_pf_structure" == *"]"* ]]; then
    legacy_pf_has_square=$((legacy_pf_has_square + 1))
  fi
  if [[ "$current_pf_structure" == *"["* || "$current_pf_structure" == *"]"* ]]; then
    current_pf_has_square=$((current_pf_has_square + 1))
  fi
  if [[ "$legacy_pf_structure" == *"("* || "$legacy_pf_structure" == *")"* ]]; then
    legacy_pf_has_round=$((legacy_pf_has_round + 1))
  fi
  if [[ "$current_pf_structure" == *"("* || "$current_pf_structure" == *")"* ]]; then
    current_pf_has_round=$((current_pf_has_round + 1))
  fi
  if [[ "$legacy_pf_energy" == "inf" || "$legacy_pf_energy" == "+inf" ]]; then
    legacy_pf_inf=$((legacy_pf_inf + 1))
  elif [[ "$legacy_pf_energy" == "nan" || "$legacy_pf_energy" == "+nan" || "$legacy_pf_energy" == "-nan" ]]; then
    legacy_pf_nan=$((legacy_pf_nan + 1))
  else
    legacy_pf_finite=$((legacy_pf_finite + 1))
  fi
  if [[ "$current_pf_energy" == "inf" || "$current_pf_energy" == "+inf" ]]; then
    current_pf_inf=$((current_pf_inf + 1))
  elif [[ "$current_pf_energy" == "nan" || "$current_pf_energy" == "+nan" || "$current_pf_energy" == "-nan" ]]; then
    current_pf_nan=$((current_pf_nan + 1))
  else
    current_pf_finite=$((current_pf_finite + 1))
  fi

  if (( mfe_ok == 1 && pf_ok == 1 )); then
    matched=$((matched + 1))
  else
    failed=$((failed + 1))
    if (( mfe_ok == 0 )); then
      mfe_failed=$((mfe_failed + 1))
    fi
    if (( pf_ok == 0 )); then
      pf_failed=$((pf_failed + 1))
      if [[ "$legacy_pf_structure" != "$current_pf_structure" ]]; then
        pf_structure_failed=$((pf_structure_failed + 1))
      fi
      if [[ "$legacy_pf_energy" != "$current_pf_energy" ]]; then
        pf_energy_failed=$((pf_energy_failed + 1))
      fi
    fi
    if (( sampled < sample_limit )); then
      echo "mismatch case_id=$case_id" >&2
      echo "legacy:  $legacy_parsed" >&2
      echo "current: $current_parsed" >&2
      sampled=$((sampled + 1))
    fi
  fi
done < "$cases_tsv"

echo "compare_total=$total"
echo "compare_matched=$matched"
echo "compare_failed=$failed"
echo "compare_skipped=$skipped"
echo "compare_mfe_failed=$mfe_failed"
echo "compare_pf_failed=$pf_failed"
echo "compare_pf_structure_failed=$pf_structure_failed"
echo "compare_pf_energy_failed=$pf_energy_failed"
echo "legacy_pf_inf=$legacy_pf_inf"
echo "legacy_pf_nan=$legacy_pf_nan"
echo "legacy_pf_finite=$legacy_pf_finite"
echo "current_pf_inf=$current_pf_inf"
echo "current_pf_nan=$current_pf_nan"
echo "current_pf_finite=$current_pf_finite"
echo "legacy_pf_has_square=$legacy_pf_has_square"
echo "legacy_pf_has_round=$legacy_pf_has_round"
echo "current_pf_has_square=$current_pf_has_square"
echo "current_pf_has_round=$current_pf_has_round"
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
