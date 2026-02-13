#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE' >&2
Usage: parse_cparty_stdout.sh [cparty_stdout_file]
Parses CParty stdout and prints: <seq>\t<restricted>\t<cli_mfe_structure>\t<cli_mfe_energy>
USAGE
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  usage
  exit 0
fi

if [[ $# -gt 1 ]]; then
  usage
  exit 2
fi

input="${1:-/dev/stdin}"
if [[ "$input" != "/dev/stdin" && ! -f "$input" ]]; then
  echo "parse error: input file not found: $input" >&2
  exit 1
fi

trimmed_lines=$(awk '{sub(/\r$/,""); if ($0 !~ /^[[:space:]]*$/) print $0}' "$input")
if [[ -z "$trimmed_lines" ]]; then
  echo "parse error: empty stdout" >&2
  exit 1
fi

seq_line=$(printf '%s\n' "$trimmed_lines" | sed -n '1p')
restricted_line=$(printf '%s\n' "$trimmed_lines" | sed -n '2p')
mfe_line=$(printf '%s\n' "$trimmed_lines" | sed -n '3p')

if [[ -z "$seq_line" || -z "$restricted_line" || -z "$mfe_line" ]]; then
  echo "parse error: missing required sequence/restricted/result lines" >&2
  exit 1
fi

if [[ ! "$seq_line" =~ ^[AUGC]+$ ]]; then
  echo "parse error: invalid sequence line: $seq_line" >&2
  exit 1
fi

restricted_invalid_chars=$(printf '%s' "$restricted_line" | tr -d '.()[]{}<>/x')
if [[ -n "$restricted_invalid_chars" ]]; then
  echo "parse error: invalid restricted structure line: $restricted_line" >&2
  exit 1
fi

if ! printf '%s\n' "$mfe_line" | grep -Eq '^([^[:space:]]+)[[:space:]]+\(([+-]?[0-9]+([.][0-9]+)?)\)$'; then
  echo "parse error: malformed MFE result line: $mfe_line" >&2
  exit 1
fi

mfe_structure=$(printf '%s\n' "$mfe_line" | sed -E 's/^([^[:space:]]+)[[:space:]]+\(([+-]?[0-9]+([.][0-9]+)?)\)$/\1/')
mfe_energy=$(printf '%s\n' "$mfe_line" | sed -E 's/^([^[:space:]]+)[[:space:]]+\(([+-]?[0-9]+([.][0-9]+)?)\)$/\2/')
mfe_invalid_chars=$(printf '%s' "$mfe_structure" | tr -d '.()[]{}<>/x')
if [[ -n "$mfe_invalid_chars" ]]; then
  echo "parse error: invalid MFE structure: $mfe_structure" >&2
  exit 1
fi

printf '%s\t%s\t%s\t%s\n' "$seq_line" "$restricted_line" "$mfe_structure" "$mfe_energy"
