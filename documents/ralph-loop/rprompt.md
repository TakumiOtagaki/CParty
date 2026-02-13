# Ralph Agent Instructions

## Context

You are running in the root of the project.
Primary planning docs are in the `documents/` directory.
Loop memory is in `documents/ralph-loop/progress.txt`.

## Your Task

1. Read `documents/prd.json`
2. Read `documents/rd.md`
3. Read `documents/ralph-loop/progress.txt` (check Codebase Patterns first)
4. Pick highest priority story where `passes: false`
5. Implement exactly one story in the current directory (`.`)
6. Run tests required by the story and full regression gate
7. Update `documents/prd.json` only when all gates are satisfied
8. Commit format: `feat: [ID] - [Title]`
9. Append learnings to `documents/ralph-loop/progress.txt`

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
