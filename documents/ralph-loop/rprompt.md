# Ralph Agent Instructions

## Context

You are running in the root of the project.
Primary planning docs are in the `documents/` directory.
Loop memory is in `documents/ralph-loop/progress.txt`.

## Your Task

1. Read `documents/ralph-loop/prd.json` (if missing, stop and report missing file; do not fabricate)
2. Read `documents/rd.md`
3. Read `documents/refactor_nonregression_checklist.md` and `documents/alignment_gate_spec.md`
4. Read `documents/ralph-loop/progress.txt` (check Codebase Patterns first)
5. Pick highest priority story where `passes: false`
6. Implement exactly one story in the current directory (`.`)
7. Run tests required by the story and full regression gate on a fresh build directory
8. Update `documents/ralph-loop/prd.json` only when all gates are satisfied
9. Commit format: `feat: [ID] - [Title]`
10. Update `documents/ralph-loop/audit/{story_id}.md` using `documents/ralph-loop/audit/TEMPLATE.md`
11. Append learnings to `documents/ralph-loop/progress.txt`

## Hard Guards (No Loopholes)

- Do not claim pass using stale `build/` artifacts; regenerate build files before running `ctest`.
- Do not treat `alignment_compared=0` as success for alignment tests.
- Do not modify tests/baselines only to force green unless explicitly required by the story.
- Do not set any story to `passes: true` without command evidence in the same run.
- For refactor-only stories, do not mark pass unless `refactor_compared` and `refactor_strict_mismatched` are reported and non-regressing.
- For alignment stories, do not mark pass unless `alignment_compared` and `alignment_mismatched` are reported.
- In refactor stories, prioritize extraction of repeated concrete operations (>=3 occurrences); avoid broad abstraction-first helpers.
- Do not mark pass unless `documents/ralph-loop/audit/{story_id}.md` is updated in the same run.
- If required files/directories are missing (e.g., `documents/ralph-loop/prd.json`, `test/`), stop and report the blocker.

## Progress Format

Append to `documents/ralph-loop/progress.txt`:

## [Date] - [Story ID]
- What was implemented
- Files changed
- **Learnings:**
  - Patterns discovered
  - Gotchas encountered
---

## Stop Condition

If all stories pass, reply with exactly:
<promise>COMPLETE</promise>
