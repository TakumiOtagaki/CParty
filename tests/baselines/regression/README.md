# Regression Matrix Baselines

Baselines are named as:

- `r{0|1}_p{0|1}_k{0|1}_d{0|1}.txt`

Where:

- `r1` means `-r "(............................)"` is applied.
- `p1` means `-p` is applied.
- `k1` means `-k` is applied.
- `d1` means `-d0` is applied; `d0` means default dangle model.

Note:

- `r0_p1_k1_d*` is intentionally excluded because `-p -k` without `-r` is currently unsupported and crashes.
