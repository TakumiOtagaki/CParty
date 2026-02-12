#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 3 || $# -gt 4 ]]; then
  echo "usage: $0 <cparty_binary> <fixture_fasta> <baseline_tsv> [--generate]" >&2
  exit 2
fi

binary="$1"
fixture_fasta="$2"
baseline_tsv="$3"
mode="${4:-}"

if [[ ! -x "$binary" ]]; then
  echo "binary is not executable: $binary" >&2
  exit 1
fi
if [[ ! -f "$fixture_fasta" ]]; then
  echo "fixture file not found: $fixture_fasta" >&2
  exit 1
fi

parse_mfe_line() {
  local text="$1"
  local line parsed structure bad_chars
  while IFS= read -r line; do
    parsed="$(printf '%s\n' "$line" | sed -nE 's/^([^[:space:]]+)[[:space:]]+\(([-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?)\)$/\1\t\2/p')"
    if [[ -n "$parsed" ]]; then
      structure="${parsed%%$'\t'*}"
      bad_chars="$(printf '%s' "$structure" | tr -d '.()[]{}<>')"
      if [[ -z "$bad_chars" ]]; then
        printf '%s\n' "$parsed"
        return 0
      fi
    fi
    parsed="$(printf '%s\n' "$line" | sed -nE 's/^Result_[0-9]+:[[:space:]]+([^[:space:]]+)[[:space:]]+\(([-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?)\)$/\1\t\2/p')"
    if [[ -n "$parsed" ]]; then
      structure="${parsed%%$'\t'*}"
      bad_chars="$(printf '%s' "$structure" | tr -d '.()[]{}<>')"
      if [[ -z "$bad_chars" ]]; then
        printf '%s\n' "$parsed"
        return 0
      fi
    fi
  done <<< "$text"
  return 1
}

parser_self_test() {
  local sample parsed
  sample=$'GCAUGC\n(....)\n...... (0)\n(....) (0)\n(....) (0)\n(....) (6)\nfrequency of MFE structure in ensemble: 1; ensemble diversity 0'
  parsed="$(parse_mfe_line "$sample")" || {
    echo "parser self-test failed: could not parse MFE line" >&2
    exit 1
  }
  if [[ "$parsed" != $'......\t0' ]]; then
    echo "parser self-test failed: expected '......\\t0', got '$parsed'" >&2
    exit 1
  fi
}

parser_self_test

declare -a fixture_ids=()
declare -a fixture_seqs=()
declare -a fixture_constraints=()

current_id=""
current_expected=""
current_seq=""
current_constraint=""

commit_fixture_if_ready() {
  if [[ -n "$current_id" && -n "$current_seq" && -n "$current_constraint" ]]; then
    if [[ "$current_expected" == "valid" ]]; then
      fixture_ids+=("$current_id")
      fixture_seqs+=("$current_seq")
      fixture_constraints+=("$current_constraint")
    fi
  fi
  current_id=""
  current_expected=""
  current_seq=""
  current_constraint=""
}

while IFS= read -r raw_line || [[ -n "$raw_line" ]]; do
  line="${raw_line%$'\r'}"
  [[ -z "$line" ]] && continue
  if [[ "$line" == ">"* ]]; then
    commit_fixture_if_ready
    header="${line#>}"
    current_id="${header%% *}"
    if [[ "$header" =~ expected=([a-zA-Z_]+) ]]; then
      current_expected="${BASH_REMATCH[1]}"
    else
      current_expected=""
    fi
    continue
  fi
  if [[ -z "$current_id" ]]; then
    echo "malformed fixture file: content before header" >&2
    exit 1
  fi
  if [[ -z "$current_seq" ]]; then
    current_seq="$line"
    continue
  fi
  if [[ -z "$current_constraint" ]]; then
    current_constraint="$line"
    continue
  fi
done < "$fixture_fasta"
commit_fixture_if_ready

if [[ "${#fixture_ids[@]}" -eq 0 ]]; then
  echo "no valid fixtures found in $fixture_fasta" >&2
  exit 1
fi

declare -a flag_ids=(
  "r1_p0_k0_d2"
  "r1_p0_k0_d0"
  "r1_p1_k0_d2"
  "r1_p1_k0_d0"
  "r1_p0_k1_d2"
  "r1_p0_k1_d0"
)
declare -a flag_values=(
  ""
  "-d0"
  "-p"
  "-p -d0"
  "-k"
  "-k -d0"
)

generate_rows() {
  local i j
  printf "fixture_id\tflag_id\tflags\tconstraint_mode\texit_code\tstatus\tmfe_structure\tmfe_energy\n"
  for ((i = 0; i < ${#fixture_ids[@]}; ++i)); do
    for ((j = 0; j < ${#flag_ids[@]}; ++j)); do
      local id seq constraint flag_id flags
      id="${fixture_ids[$i]}"
      seq="${fixture_seqs[$i]}"
      constraint="${fixture_constraints[$i]}"
      flag_id="${flag_ids[$j]}"
      flags="${flag_values[$j]}"

      local cmd=("$binary")
      if [[ -n "$flags" ]]; then
        read -r -a split_flags <<< "$flags"
        cmd+=("${split_flags[@]}")
      fi
      cmd+=("-r" "$constraint" "$seq")

      local out1 out2 ec1 ec2 parsed mfe_structure mfe_energy status
      set +e
      out1="$("${cmd[@]}" 2>&1)"
      ec1=$?
      out2="$("${cmd[@]}" 2>&1)"
      ec2=$?
      set -e

      if [[ "$ec1" -ne "$ec2" || "$out1" != "$out2" ]]; then
        echo "non-deterministic CLI output for fixture=$id flags='$flags'" >&2
        exit 1
      fi

      if [[ "$ec1" -eq 0 ]]; then
        parsed="$(parse_mfe_line "$out1")" || {
          echo "failed to parse MFE output for fixture=$id flags='$flags'" >&2
          exit 1
        }
        mfe_structure="${parsed%%$'\t'*}"
        mfe_energy="${parsed#*$'\t'}"
        status="ok"
      else
        mfe_structure=""
        mfe_energy=""
        status="error"
      fi

      printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n" \
        "$id" "$flag_id" "$flags" "r" "$ec1" "$status" "$mfe_structure" "$mfe_energy"
    done
  done
}

if [[ "$mode" == "--generate" ]]; then
  mkdir -p "$(dirname "$baseline_tsv")"
  generate_rows > "$baseline_tsv"
  exit 0
fi

if [[ ! -f "$baseline_tsv" ]]; then
  echo "baseline file not found: $baseline_tsv" >&2
  exit 1
fi

temp_output="$(mktemp)"
trap 'rm -f "$temp_output"' EXIT
generate_rows > "$temp_output"

if ! diff -u "$baseline_tsv" "$temp_output"; then
  echo "fixed-energy CLI baseline mismatch" >&2
  exit 1
fi
