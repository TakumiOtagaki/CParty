#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "usage: $0 <fixtures_tsv>" >&2
  exit 2
fi

fixtures_tsv="$1"
if [[ ! -f "$fixtures_tsv" ]]; then
  echo "fixtures file not found: $fixtures_tsv" >&2
  exit 1
fi

fail() {
  echo "fixture validation failed: $1" >&2
  exit 1
}

contains_char() {
  local text="$1"
  local chars="$2"
  local i ch
  for ((i = 0; i < ${#text}; i++)); do
    ch="${text:i:1}"
    if [[ "$chars" == *"$ch"* ]]; then
      return 0
    fi
  done
  return 1
}

count_pk_families() {
  local db="$1"
  local count=0
  contains_char "$db" "[]" && count=$((count + 1))
  contains_char "$db" "{}" && count=$((count + 1))
  contains_char "$db" "<>" && count=$((count + 1))
  printf '%s' "$count"
}

check_bracket_balance() {
  local db="$1"
  local stack_paren=""
  local stack_sq=""
  local stack_curly=""
  local stack_angle=""
  local i ch

  for ((i = 0; i < ${#db}; i++)); do
    ch="${db:i:1}"
    case "$ch" in
      '(') stack_paren+="(" ;;
      ')')
        [[ -n "$stack_paren" ]] || return 1
        stack_paren="${stack_paren%?}"
        ;;
      '[') stack_sq+="[" ;;
      ']')
        [[ -n "$stack_sq" ]] || return 1
        stack_sq="${stack_sq%?}"
        ;;
      '{') stack_curly+="{" ;;
      '}')
        [[ -n "$stack_curly" ]] || return 1
        stack_curly="${stack_curly%?}"
        ;;
      '<') stack_angle+="<" ;;
      '>')
        [[ -n "$stack_angle" ]] || return 1
        stack_angle="${stack_angle%?}"
        ;;
      '.') ;;
      *) return 1 ;;
    esac
  done

  [[ -z "$stack_paren" && -z "$stack_sq" && -z "$stack_curly" && -z "$stack_angle" ]]
}

valid_case_violation() {
  local family="$1"
  local seq="$2"
  local db="$3"
  local len_seq="${#seq}"
  local len_db="${#db}"
  local pk_count i ch

  [[ "$seq" =~ ^[ACGTU]+$ ]] || { echo "invalid sequence alphabet"; return 0; }
  for ((i = 0; i < ${#db}; i++)); do
    ch="${db:i:1}"
    contains_char "$ch" ".()[]{}<>" || { echo "invalid structure symbols"; return 0; }
  done
  [[ "$len_seq" -eq "$len_db" ]] || { echo "length mismatch"; return 0; }
  check_bracket_balance "$db" || { echo "unbalanced structure"; return 0; }

  pk_count="$(count_pk_families "$db")"

  case "$family" in
    pk_free)
      [[ "$db" =~ ^[\.\(\)]+$ ]] || { echo "pk_free contains PK brackets"; return 0; }
      ;;
    h_type|k_type)
      contains_char "$db" "[]" || { echo "$family missing [] PK brackets"; return 0; }
      [[ "$pk_count" -eq 1 ]] || { echo "$family mixes PK families"; return 0; }
      [[ "$db" =~ [\{\}\<\>] ]] && { echo "$family uses unsupported PK family"; return 0; }
      ;;
    *)
      echo "unknown family for valid fixture"
      return 0
      ;;
  esac

  return 1
}

validate_invalid_reason() {
  local reason="$1"
  local seq="$2"
  local db="$3"
  local pk_count

  case "$reason" in
    length_mismatch)
      [[ "${#seq}" -ne "${#db}" ]]
      ;;
    invalid_sequence_symbol)
      [[ ! "$seq" =~ ^[ACGTU]+$ ]]
      ;;
    unbalanced_structure)
      ! check_bracket_balance "$db"
      ;;
    mixed_pk_families)
      pk_count="$(count_pk_families "$db")"
      [[ "$pk_count" -ge 2 ]]
      ;;
    unsupported_pk_family)
      [[ "$db" =~ [\{\}\<\>] ]]
      ;;
    *)
      return 1
      ;;
  esac
}

seen_ids_file="$(mktemp)"
trap 'rm -f "$seen_ids_file"' EXIT

line_no=0
while IFS=$'\t' read -r id family expected seq db reason rest; do
  line_no=$((line_no + 1))

  [[ -n "${id}${family}${expected}${seq}${db}${reason}" ]] || continue
  [[ "$id" == \#* ]] && continue

  [[ -z "$rest" ]] || fail "line ${line_no}: too many tab-separated fields"
  [[ -n "$id" && -n "$family" && -n "$expected" && -n "$seq" && -n "$db" && -n "$reason" ]] || fail "line ${line_no}: missing required field"

  if grep -Fxq "$id" "$seen_ids_file"; then
    fail "line ${line_no}: duplicate id '$id'"
  fi
  printf '%s\n' "$id" >>"$seen_ids_file"

  [[ "$expected" == "valid" || "$expected" == "invalid" ]] || fail "line ${line_no}: expected must be valid|invalid"
  [[ "$family" == "pk_free" || "$family" == "h_type" || "$family" == "k_type" || "$family" == "invalid" ]] || fail "line ${line_no}: unsupported family '$family'"

  if [[ "$expected" == "valid" ]]; then
    [[ "$family" != "invalid" ]] || fail "line ${line_no}: valid case cannot use family=invalid"
    [[ "$reason" == "-" ]] || fail "line ${line_no}: valid case must use invalid_reason '-'"

    if violation="$(valid_case_violation "$family" "$seq" "$db")"; then
      fail "line ${line_no}: valid case '$id' is invalid: ${violation}"
    fi
  else
    [[ "$family" == "invalid" ]] || fail "line ${line_no}: invalid case must use family=invalid"
    [[ "$reason" != "-" ]] || fail "line ${line_no}: invalid case must provide reason tag"

    validate_invalid_reason "$reason" "$seq" "$db" || fail "line ${line_no}: invalid case '$id' does not match reason '${reason}'"
  fi

done <"$fixtures_tsv"

# Ensure every required valid family is represented at least once.
for required_family in pk_free h_type k_type; do
  if ! awk -F '\t' -v fam="$required_family" 'NF >= 6 && $1 !~ /^#/ && $2 == fam && $3 == "valid" { found=1 } END { exit(found ? 0 : 1) }' "$fixtures_tsv"; then
    fail "missing required valid family '$required_family'"
  fi
done

echo "validated fixed-energy fixtures: $fixtures_tsv"
