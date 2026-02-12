# CParty Fixed-Structure Energy API (Draft Requirements)

## 1. Purpose
Implement an API that returns the fixed-structure energy **E(G∪G', S)** in **kcal/mol**. This enables exact conditional probability:

```
logP(G' | G, S) = -E(G∪G', S) / RT - logZ_cond
```

## 2. Scope
- Add a CParty internal function to evaluate the energy of a **given dot-bracket structure** (with PK brackets) for a sequence.
- Expose a **public API** in `CPartyAPI` returning that energy.
- Do not remove or change existing APIs.

## 3. Inputs / Outputs
### Inputs
- `seq`: RNA sequence (ACGU or T treated as U)
- `db_full`: dot-bracket of the full structure **G∪G'** (should allow PK brackets)

### Output
- Energy `E(G∪G', S)` in **kcal/mol** (double)
- Return **NaN** on invalid input (length mismatch, invalid chars, invalid structure)

## 4. Implementation Notes
- Reuse CParty's existing energy routines when possible.
- Ensure Turner 2004 parameters are loaded (consistent with existing API).
- This is **fixed-structure evaluation**, not MFE.

## 5. Testing
- Energy is finite for valid inputs.
- Deterministic: same inputs -> same energy.
- Invalid inputs return NaN.

## 6. Follow-up
- Use this energy with `logZ_cond` to compute exact `logP(G'|G,S)` in the main project.
