#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 5 ]]; then
  echo "usage: $0 <ctest_bin> <build_dir> <cparty_binary> <fixture_fasta> <report_md>" >&2
  exit 2
fi

ctest_bin="$1"
build_dir="$2"
binary="$3"
fixture_fasta="$4"
report_md="$5"

if [[ ! -x "$ctest_bin" ]]; then
  echo "ctest is not executable: $ctest_bin" >&2
  exit 1
fi
if [[ ! -d "$build_dir" ]]; then
  echo "build directory not found: $build_dir" >&2
  exit 1
fi
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
  done <<< "$text"
  return 1
}

assert_finite_number() {
  local value="$1"
  local lower
  local num_re='^[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?$'
  lower="$(printf '%s' "$value" | tr '[:upper:]' '[:lower:]')"
  if [[ ! "$value" =~ $num_re ]] || [[ "$lower" == *nan* ]] || [[ "$lower" == *inf* ]]; then
    return 1
  fi
  return 0
}

run_smoke_case() {
  local case_id="$1"
  local flags="$2"
  local constraint="$3"
  local seq="$4"
  local max_seconds="$5"

  local cmd=("$binary")
  if [[ -n "$flags" ]]; then
    read -r -a split_flags <<< "$flags"
    cmd+=("${split_flags[@]}")
  fi
  if [[ -n "$constraint" ]]; then
    cmd+=("-r" "$constraint")
  fi
  cmd+=("$seq")

  local start end duration out1 out2 ec1 ec2 parsed energy
  start=$(date +%s)
  set +e
  out1="$("${cmd[@]}" 2>&1)"
  ec1=$?
  out2="$("${cmd[@]}" 2>&1)"
  ec2=$?
  set -e
  end=$(date +%s)
  duration=$((end - start))

  if [[ "$ec1" -ne 0 || "$ec2" -ne 0 ]]; then
    echo "smoke case failed for ${case_id}: ${cmd[*]}" >&2
    exit 1
  fi
  if [[ "$out1" != "$out2" ]]; then
    echo "non-deterministic smoke output for ${case_id}" >&2
    exit 1
  fi
  if (( duration > max_seconds )); then
    echo "smoke case exceeded time budget (${duration}s > ${max_seconds}s) for ${case_id}" >&2
    exit 1
  fi

  parsed="$(parse_mfe_line "$out1")" || {
    echo "failed to parse MFE output for smoke case ${case_id}" >&2
    exit 1
  }
  energy="${parsed#*$'\t'}"
  if ! assert_finite_number "$energy"; then
    echo "non-finite MFE energy in smoke case ${case_id}: ${energy}" >&2
    exit 1
  fi

  printf "%s\t%s\t%s\n" "$case_id" "$duration" "$energy"
}

integration_pattern='^(regression_matrix|can_pair_policy|pseudo_loop_can_pair_integration|pseudo_loop_can_pair_integration_helpers|part_func_can_pair_integration|part_func_can_pair_integration_helpers|can_pair_cross_path_consistency)$'
(
  cd "$build_dir"
  "$ctest_bin" --output-on-failure -R "$integration_pattern"
)

# Representative smoke inputs: one PK-free and two density-2 constrained PK cases.
smoke_rows=()
smoke_rows+=("$(run_smoke_case "pkfree_r" "" "(............................)" "GCAACGAUGACAUACAUCGCUAGUCGACGC" 30)")
smoke_rows+=("$(run_smoke_case "h_type_r_k" "-k" "((((....[[[[....))))....]]]]" "GGGCAAAAGGCGAAAAGCCCAAAACGCC" 30)")
smoke_rows+=("$(run_smoke_case "k_type_r_k_d0" "-k -d0" "(((((.....[[[[[.....))))).....(((((.....]]]]].....)))))" "GGCGGAAAAACCGGCAAAAACCGCCAAAAACGGGUAAUAAGCUGGAAAAAGCCCG" 30)")

mkdir -p "$(dirname "$report_md")"
{
  printf "# Can-Pair Rollout Report\n\n"
  printf "## Scope\n"
  printf "This report summarizes final can-pair rollout verification across fixed-energy API, pseudo-loop, and partition-function paths.\n\n"
  printf "## Changed Decision Points\n"
  printf '%s\n' '- `src/can_pair_policy.hh` and `src/can_pair_policy.cc`: shared can-pair hard-constraint policy for allowed base pairs and tree-up span checks.'
  printf '%s\n' '- `src/pseudo_loop_can_pair.hh` and `src/pseudo_loop_can_pair.cc`: pseudo-loop recurrence/backtracking pair gating routed through shared policy helpers.'
  printf '%s\n' '- `src/part_func_can_pair.hh` and `src/part_func_can_pair.cc`: partition-function recurrence/backtracking pair gating routed through shared policy helpers.'
  printf '%s\n\n' '- `tests/can_pair_cross_path_consistency_test.cc`: one shared acceptance/rejection matrix asserting API/helper outcome alignment.'
  printf "## Regression and Integration Gate\n"
  printf "The following tests were executed together in one gate:\n"
  printf '%s\n' '- `regression_matrix`'
  printf '%s\n' '- `can_pair_policy`'
  printf '%s\n' '- `pseudo_loop_can_pair_integration`'
  printf '%s\n' '- `pseudo_loop_can_pair_integration_helpers`'
  printf '%s\n' '- `part_func_can_pair_integration`'
  printf '%s\n' '- `part_func_can_pair_integration_helpers`'
  printf '%s\n\n' '- `can_pair_cross_path_consistency`'
  printf "## Performance Smoke Checks\n"
  printf "| case | elapsed_seconds | parsed_mfe_energy |\n"
  printf "| --- | ---: | ---: |\n"
  for row in "${smoke_rows[@]}"; do
    case_id="${row%%$'\t'*}"
    rest="${row#*$'\t'}"
    elapsed="${rest%%$'\t'*}"
    energy="${rest#*$'\t'}"
    printf "| %s | %s | %s |\n" "$case_id" "$elapsed" "$energy"
  done
  printf "\n## Remaining Limitations\n"
  printf '%s\n' '- CLI constrained `-k` handling still has known deterministic failure modes for some fixture/flag combinations, tracked in `tests/baselines/fixed_energy_cli/density2_mfe.tsv`.'
  printf '%s\n' '- Representability remains bounded by current CParty grammar; unsupported structures are intentionally rejected (`NaN`) by fixed-energy API validation.'
  printf '%s\n' '- Smoke timing here is a sanity gate on representative inputs, not a full benchmark suite.'
} > "$report_md"
