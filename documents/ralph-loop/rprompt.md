# Ralph Agent Instructions

## Context

You are running in the root of the project.
Primary planning docs are in the `documents/` directory.
Loop memory is in `documents/ralph-loop/progress.txt`.

## Your Task

1. Read `documents/ralph-loop/prd.json` (if missing, stop and report missing file; do not fabricate)
2. Read `documents/rd.md`
3. Read `documents/ralph-loop/progress.txt` (check Codebase Patterns first)
4. Pick highest priority story where `passes: false`
5. Implement exactly one story in the current directory (`.`)
6. Run tests required by the story and full regression gate on a fresh build directory
7. Update `documents/ralph-loop/prd.json` only when all gates are satisfied
8. Commit format: `feat: [ID] - [Title]`
9. Update `documents/ralph-loop/audit/{story_id}.md` with commands, metrics, and result
10. Append learnings to `documents/ralph-loop/progress.txt`

## Hard Guards (No Loopholes)

- Do not claim pass using stale `build/` artifacts; regenerate build files before running `ctest`.
- Do not treat `alignment_compared=0` as success for alignment tests.
- Do not modify tests/baselines only to force green unless explicitly required by the story.
- Do not set any story to `passes: true` without command evidence in the same run.
- For refactor-only stories, do not mark pass unless `refactor_compared` and `refactor_strict_mismatched` are reported and non-regressing.
- For alignment stories, do not mark pass unless `alignment_compared` and `alignment_mismatched` are reported.
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
