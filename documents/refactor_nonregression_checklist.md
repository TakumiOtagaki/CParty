# Refactor Non-Regression Checklist

## Scope
このチェックリストは refactor-only story (`007-010`) で必須。

## Preconditions
- [ ] Fresh build was executed:
  `rm -rf build && cmake -S . -B build && cmake --build build`
- [ ] Story audit file exists: `documents/ralph-loop/audit/{story_id}.md`

## No-Behavior-Change Rules
- [ ] No recurrence formula changes
- [ ] No baseline editing only to force green
- [ ] No stale build artifacts used

## Required Commands
- [ ] Story-specific tests executed
- [ ] Full regression executed: `ctest --test-dir build --output-on-failure`

## Required Metrics
- [ ] `refactor_compared` reported
- [ ] `refactor_strict_mismatched` reported
- [ ] `refactor_compared_current == refactor_compared_previous`
- [ ] `refactor_strict_mismatched_current <= refactor_strict_mismatched_previous`

## Pass/Fail Rule
- PASS only if all checks above are true
- Otherwise keep `passes=false` and write blocker in audit log

## Fixed-Energy Rollout Registry (Story 013)
- Canonical 16-state registry must stay synchronized between docs and code.
- Story mapping source of truth is `fixed_energy_rollout_plan()`.
- Slice test names must remain source-registered:
  - `fixed_energy_slice_a`
  - `fixed_energy_slice_b`
  - `fixed_energy_slice_c`
  - `fixed_energy_slice_d`
