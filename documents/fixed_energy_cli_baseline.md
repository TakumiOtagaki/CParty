# Fixed-Energy CLI Baseline (Density-2)

This baseline captures CParty CLI density-2 MFE outputs generated from:

- Fixture source: `tests/cparty_fixed_structure_energy/test.multi.fa`
- Fixtures included: entries marked `expected=valid`
- Constraint mode: always `-r <fixture_structure>`
- Flag variants:
  - `r1_p0_k0_d2`
  - `r1_p0_k0_d0`
  - `r1_p1_k0_d2`
  - `r1_p1_k0_d0`
  - `r1_p0_k1_d2`
  - `r1_p0_k1_d0`

Stored dataset:

- `tests/baselines/fixed_energy_cli/density2_mfe.tsv`

Columns:

- `fixture_id`
- `flag_id`
- `flags`
- `constraint_mode`
- `exit_code`
- `status` (`ok` or `error`)
- `mfe_structure`
- `mfe_energy`

Regeneration command:

```bash
tests/fixed_energy_cli_baseline.sh build/CParty \
  tests/cparty_fixed_structure_energy/test.multi.fa \
  tests/baselines/fixed_energy_cli/density2_mfe.tsv \
  --generate
```

Validation command:

```bash
ctest --output-on-failure -R fixed_energy_cli_baseline
```
