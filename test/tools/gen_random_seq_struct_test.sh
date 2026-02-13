#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "$0")/../.." && pwd)
GEN="$ROOT_DIR/test/tools/gen_random_seq_struct.py"

if [[ ! -x "$GEN" ]]; then
  echo "missing executable generator: $GEN" >&2
  exit 1
fi

TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

OUT1="$TMP_DIR/out1.tsv"
OUT2="$TMP_DIR/out2.tsv"
OUT3="$TMP_DIR/out3.tsv"

python3 "$GEN" --seed 123 --count 50 --length 36 --prefix s > "$OUT1"
python3 "$GEN" --seed 123 --count 50 --length 36 --prefix s > "$OUT2"
python3 "$GEN" --seed 124 --count 50 --length 36 --prefix s > "$OUT3"

if ! cmp -s "$OUT1" "$OUT2"; then
  echo "determinism check failed: same seed produced different output" >&2
  exit 1
fi

if cmp -s "$OUT1" "$OUT3"; then
  echo "randomness check failed: different seed produced identical output" >&2
  exit 1
fi

python3 - "$OUT1" <<'PY'
import csv
import re
import sys

path = sys.argv[1]
allowed = {("A", "U"), ("U", "A"), ("G", "C"), ("C", "G"), ("G", "U"), ("U", "G")}

with open(path, newline="", encoding="utf-8") as fh:
    reader = csv.DictReader(fh, delimiter="\t")
    if reader.fieldnames != ["case_id", "seq", "G"]:
        raise SystemExit(f"unexpected header: {reader.fieldnames}")

    rows = list(reader)
    if len(rows) != 50:
        raise SystemExit(f"expected 50 rows, got {len(rows)}")

    for idx, row in enumerate(rows, start=1):
        case_id, seq, struct = row["case_id"], row["seq"], row["G"]
        if case_id != f"s_{idx:04d}":
            raise SystemExit(f"unexpected case id order: {case_id}")
        if len(seq) != 36 or len(struct) != 36:
            raise SystemExit("length mismatch")
        if re.search(r"[^AUGC]", seq):
            raise SystemExit(f"invalid sequence alphabet in {case_id}: {seq}")
        if re.search(r"[^().]", struct):
            raise SystemExit(f"invalid structure chars in {case_id}: {struct}")

        stack = []
        for i, ch in enumerate(struct):
            if ch == "(":
                stack.append(i)
            elif ch == ")":
                if not stack:
                    raise SystemExit(f"unbalanced close paren in {case_id}")
                j = stack.pop()
                if (seq[j], seq[i]) not in allowed:
                    raise SystemExit(
                        f"disallowed pair for {case_id} at ({j},{i}): {(seq[j], seq[i])}"
                    )
        if stack:
            raise SystemExit(f"unbalanced open paren in {case_id}")

print("gen_random_seq_struct_test: PASS")
PY
