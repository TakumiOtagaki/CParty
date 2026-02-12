#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
  echo "usage: $0 <cparty_binary> <baseline_dir>" >&2
  exit 2
fi

binary="$1"
baseline_dir="$2"

if [[ ! -x "$binary" ]]; then
  echo "binary is not executable: $binary" >&2
  exit 1
fi

if [[ ! -d "$baseline_dir" ]]; then
  echo "baseline directory not found: $baseline_dir" >&2
  exit 1
fi

seq="GCAACGAUGACAUACAUCGCUAGUCGACGC"
constraint="(............................)"

assert_finite_output() {
  local text="$1"
  local energies summary_line freq diversity value lower
  local num_re='^[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?$'

  energies="$(printf '%s\n' "$text" | grep -Eo '\([-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?\)' || true)"
  if [[ -z "$energies" ]]; then
    echo "no energy values found in output" >&2
    exit 1
  fi

  while IFS= read -r value; do
    value="${value#(}"
    value="${value%)}"
    lower="$(printf '%s' "$value" | tr '[:upper:]' '[:lower:]')"
    if [[ ! "$value" =~ $num_re ]] || [[ "$lower" == *nan* ]] || [[ "$lower" == *inf* ]]; then
      echo "non-finite energy value: $value" >&2
      exit 1
    fi
  done <<<"$energies"

  summary_line="$(printf '%s\n' "$text" | grep 'frequency of MFE structure in ensemble:' | tail -n 1 || true)"
  if [[ -z "$summary_line" ]]; then
    echo "missing ensemble summary line" >&2
    exit 1
  fi

  freq="$(printf '%s\n' "$summary_line" | sed -E 's/.*frequency of MFE structure in ensemble:[[:space:]]*([^;]+);.*/\1/' | xargs)"
  diversity="$(printf '%s\n' "$summary_line" | sed -E 's/.*ensemble diversity[[:space:]]*([^[:space:]]+).*/\1/' | xargs)"

  for value in "$freq" "$diversity"; do
    lower="$(printf '%s' "$value" | tr '[:upper:]' '[:lower:]')"
    if [[ ! "$value" =~ $num_re ]] || [[ "$lower" == *nan* ]] || [[ "$lower" == *inf* ]]; then
      echo "non-finite summary value: $value" >&2
      exit 1
    fi
  done
}

run_case() {
  local case_id="$1"
  local use_pk_free="$2"
  local use_pk_only="$3"
  local use_dangles="$4"
  local use_constraint="$5"

  local cmd=("$binary")
  if [[ "$use_pk_free" == "1" ]]; then
    cmd+=("-p")
  fi
  if [[ "$use_pk_only" == "1" ]]; then
    cmd+=("-k")
  fi
  if [[ "$use_dangles" == "1" ]]; then
    cmd+=("-d0")
  fi
  if [[ "$use_constraint" == "1" ]]; then
    cmd+=("-r" "$constraint")
  fi
  cmd+=("$seq")

  local output_first output_second baseline_file temp_output
  if ! output_first="$("${cmd[@]}")"; then
    echo "command failed for case ${case_id}: ${cmd[*]}" >&2
    exit 1
  fi
  if ! output_second="$("${cmd[@]}")"; then
    echo "second run failed for case ${case_id}: ${cmd[*]}" >&2
    exit 1
  fi

  if [[ "$output_first" != "$output_second" ]]; then
    echo "determinism check failed for case ${case_id}" >&2
    exit 1
  fi

  assert_finite_output "$output_first"

  baseline_file="${baseline_dir}/${case_id}.txt"
  if [[ ! -f "$baseline_file" ]]; then
    echo "missing baseline file: ${baseline_file}" >&2
    exit 1
  fi

  temp_output="$(mktemp)"
  printf '%s\n' "$output_first" >"$temp_output"
  if ! diff -u "$baseline_file" "$temp_output"; then
    rm -f "$temp_output"
    echo "baseline mismatch for case ${case_id}" >&2
    exit 1
  fi
  rm -f "$temp_output"
}

for r in 0 1; do
  for p in 0 1; do
    for k in 0 1; do
      for d in 0 1; do
        if [[ "$r" == "0" && "$p" == "1" && "$k" == "1" ]]; then
          # This flag pair is only valid when a constraint is provided.
          continue
        fi
        case_id="r${r}_p${p}_k${k}_d${d}"
        run_case "$case_id" "$p" "$k" "$d" "$r"
      done
    done
  done
done
