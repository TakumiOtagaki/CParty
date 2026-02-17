#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
PARSER="$ROOT_DIR/test/tools/parse_cparty_stdout.sh"
CPARTY_BIN="${CPARTY_BIN:-$ROOT_DIR/build/CParty}"

if [[ ! -x "$PARSER" ]]; then
  echo "missing parser script: $PARSER" >&2
  exit 1
fi

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

assert_eq() {
  local got="$1"
  local expected="$2"
  local msg="$3"
  if [[ "$got" != "$expected" ]]; then
    echo "assertion failed: $msg" >&2
    echo "expected: $expected" >&2
    echo "got:      $got" >&2
    exit 1
  fi
}

assert_fail() {
  local file="$1"
  if "$PARSER" "$file" >/dev/null 2>&1; then
    echo "expected parser failure for $file" >&2
    exit 1
  fi
}

cat > "$tmpdir/valid.txt" <<'OUT'
CGGCAACAGCCG
((((....))))
((((....)))) (-3.45)
((((....)))) (-3.45)
((((....)))) (0)
OUT

parsed=$("$PARSER" "$tmpdir/valid.txt")
assert_eq "$parsed" $'CGGCAACAGCCG\t((((....))))\t((((....))))\t-3.45\t((((....))))\t-3.45' "parses valid static stdout"

cat > "$tmpdir/missing_result.txt" <<'OUT'
CGGCAACAGCCG
((((....))))
OUT
assert_fail "$tmpdir/missing_result.txt"

cat > "$tmpdir/bad_result.txt" <<'OUT'
CGGCAACAGCCG
((((....))))
((((....)))) -3.45
OUT
assert_fail "$tmpdir/bad_result.txt"

cat > "$tmpdir/bad_sequence.txt" <<'OUT'
GCXAC
(..).
(..). (0)
OUT
assert_fail "$tmpdir/bad_sequence.txt"

# Integration check against actual binary output when available.
if [[ -x "$CPARTY_BIN" ]]; then
  seq=$(awk 'BEGIN{RS="";FS="\n"} NR==4{print $2}' "$ROOT_DIR/test/multi.secstruct")
  st=$(awk 'BEGIN{RS="";FS="\n"} NR==4{print $3}' "$ROOT_DIR/test/multi.secstruct")
  "$CPARTY_BIN" -d2 -r "$st" "$seq" > "$tmpdir/live.txt" 2>/dev/null
  live_parsed=$("$PARSER" "$tmpdir/live.txt")
  live_seq=$(printf '%s' "$live_parsed" | awk -F $'\t' '{print $1}')
  live_restricted=$(printf '%s' "$live_parsed" | awk -F $'\t' '{print $2}')
  live_mfe_struct=$(printf '%s' "$live_parsed" | awk -F $'\t' '{print $3}')
  live_mfe_energy=$(printf '%s' "$live_parsed" | awk -F $'\t' '{print $4}')
  live_pf_struct=$(printf '%s' "$live_parsed" | awk -F $'\t' '{print $5}')
  live_pf_energy=$(printf '%s' "$live_parsed" | awk -F $'\t' '{print $6}')
  assert_eq "$live_seq" "CGGCAACAGCCG" "live seq matches"
  assert_eq "$live_restricted" "((((....))))" "live restricted matches"
  assert_eq "$live_mfe_struct" "((((....))))" "live mfe structure matches"
  assert_eq "$live_mfe_energy" "-3.45" "live mfe energy matches"
  if [[ -z "$live_pf_struct" || -z "$live_pf_energy" ]]; then
    echo "assertion failed: live PF fields missing" >&2
    exit 1
  fi
fi

echo "parse_cparty_stdout_test: PASS"
