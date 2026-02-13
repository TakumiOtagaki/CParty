#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE' >&2
Usage: build_density2_cli_baseline.sh <cparty_bin> <multi_secstruct_path> <output_tsv>

Builds a density-2 CLI baseline TSV with columns:
  case_id  seq  G  cli_mfe_structure  cli_mfe_energy

It processes all mandatory seed rows from multi.secstruct and appends deterministic
randomly generated valid cases.
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

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
CPARTY_BIN="$1"
SEED_DATASET="$2"
OUT_TSV="$3"
PARSER="$ROOT_DIR/test/tools/parse_cparty_stdout.sh"

RANDOM_COUNT="${BASELINE_RANDOM_COUNT:-140}"
RANDOM_SEED="${BASELINE_RANDOM_SEED:-20260213}"

if [[ ! -x "$CPARTY_BIN" ]]; then
  echo "error: missing executable CParty binary: $CPARTY_BIN" >&2
  exit 1
fi
if [[ ! -f "$SEED_DATASET" ]]; then
  echo "error: seed dataset not found: $SEED_DATASET" >&2
  exit 1
fi
if [[ ! -x "$PARSER" ]]; then
  echo "error: missing parser script: $PARSER" >&2
  exit 1
fi

mkdir -p "$(dirname "$OUT_TSV")"

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

mandatory_tsv="$tmpdir/mandatory.tsv"
random_tsv="$tmpdir/random.tsv"
cases_tsv="$tmpdir/cases.tsv"
stdout_file="$tmpdir/cparty_stdout.txt"

# Parse records as blank-line-separated blocks: header, sequence, structure.
awk '
function trim(s) { gsub(/^[[:space:]]+|[[:space:]]+$/, "", s); return s }
BEGIN { RS=""; FS="\n"; OFS="\t"; print "case_id", "seq", "G" }
{
  header = ""; seq = ""; st = "";
  for (i = 1; i <= NF; ++i) {
    line = trim($i);
    if (line == "") {
      continue;
    }
    if (header == "") { header = line; continue; }
    if (seq == "") { seq = line; continue; }
    if (st == "") { st = line; break; }
  }
  if (header == "" || seq == "" || st == "") {
    next;
  }
  if (substr(header, 1, 1) != ">") {
    next;
  }
  id = trim(substr(header, 2));
  split(id, parts, /\|/);
  id = trim(parts[1]);
  if (id == "") {
    next;
  }
  print id, seq, st;
}
' "$SEED_DATASET" > "$mandatory_tsv"

python3 - "$SEED_DATASET" "$RANDOM_SEED" "$RANDOM_COUNT" > "$random_tsv" <<'PY'
import csv
import random
import re
import sys

seed_dataset = sys.argv[1]
rng_seed = int(sys.argv[2])
random_count = int(sys.argv[3])

allowed_pairs = [("A", "U"), ("U", "A"), ("G", "C"), ("C", "G"), ("G", "U"), ("U", "G")]
alphabet = ("A", "U", "G", "C")

records = []
with open(seed_dataset, encoding="utf-8") as fh:
    chunks = fh.read().strip().split("\n\n")
for chunk in chunks:
    lines = [line.strip() for line in chunk.splitlines() if line.strip()]
    if len(lines) < 3 or not lines[0].startswith(">"):
        continue
    header = lines[0][1:]
    seq = lines[1]
    structure = lines[2]

    metadata = {}
    for token in header.split("|")[1:]:
        if "=" not in token:
            continue
        key, value = token.split("=", 1)
        metadata[key.strip()] = value.strip()

    if metadata.get("expected") != "valid":
        continue
    if metadata.get("family") != "pk_free":
        continue
    if re.search(r"[^().]", structure):
        continue

    case_id = header.split("|", 1)[0].strip()
    records.append((case_id, seq, structure))

if not records:
    raise SystemExit("error: no valid pk_free templates found in seed dataset")

rng = random.Random(rng_seed)

def randomize_sequence(structure: str) -> str:
    seq = ["?"] * len(structure)
    stack = []
    for i, ch in enumerate(structure):
        if ch == "(":
            stack.append(i)
        elif ch == ")":
            if not stack:
                raise ValueError("unbalanced structure")
            j = stack.pop()
            b1, b2 = rng.choice(allowed_pairs)
            seq[j] = b1
            seq[i] = b2
        elif ch == ".":
            seq[i] = rng.choice(alphabet)
        else:
            raise ValueError(f"unsupported char: {ch}")
    if stack:
        raise ValueError("unbalanced structure")
    return "".join(seq)

writer = csv.writer(sys.stdout, delimiter="\t", lineterminator="\n")
writer.writerow(["case_id", "seq", "G"])
for i in range(1, random_count + 1):
    template_id, _, structure = rng.choice(records)
    seq = randomize_sequence(structure)
    writer.writerow([f"rand_{i:04d}_{template_id}", seq, structure])
PY

cat "$mandatory_tsv" > "$cases_tsv"
tail -n +2 "$random_tsv" >> "$cases_tsv"

printf 'case_id\tseq\tG\tcli_mfe_structure\tcli_mfe_energy\n' > "$OUT_TSV"

total_cases=0
compared=0
skipped=0

while IFS=$'\t' read -r case_id seq g; do
  if [[ "$case_id" == "case_id" ]]; then
    continue
  fi
  total_cases=$((total_cases + 1))

  if ! "$CPARTY_BIN" -d2 -r "$g" "$seq" > "$stdout_file" 2>/dev/null; then
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

  if [[ -z "$cli_seq" || -z "$cli_restricted" || -z "$cli_mfe_structure" || -z "$cli_mfe_energy" ]]; then
    skipped=$((skipped + 1))
    continue
  fi

  printf '%s\t%s\t%s\t%s\t%s\n' "$case_id" "$seq" "$g" "$cli_mfe_structure" "$cli_mfe_energy" >> "$OUT_TSV"
  compared=$((compared + 1))
done < "$cases_tsv"

if (( compared < 100 )); then
  echo "error: insufficient compared rows: $compared (<100), skipped=$skipped, total_cases=$total_cases" >&2
  exit 1
fi

echo "baseline_total_cases=$total_cases"
echo "baseline_compared=$compared"
echo "baseline_skipped=$skipped"
echo "baseline_output=$OUT_TSV"
