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
assert_eq "$parsed" $'CGGCAACAGCCG\t((((....))))\t((((....))))\t-3.45' "parses valid static stdout"

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
  expected_live=$'CGGCAACAGCCG\t((((....))))\t((((....))))\t-3.45'
  assert_eq "$live_parsed" "$expected_live" "parses live CParty stdout deterministically"
fi

echo "parse_cparty_stdout_test: PASS"
