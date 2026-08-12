# R002 — AES S-box Circuit Optimization

**Unsolved Labs Research Release R002**

An explicit proof-carrying optimization of a 29-AND forward AES S-box circuit from the NIST circuit repository.

## Result

Pinned NIST baseline:

- 29 AND gates
- 97 XOR gates
- 14 XNOR gates
- 140 total gates
- gate depth 37
- AND-depth 5

Released candidate:

- 29 AND gates
- 97 XOR gates
- 13 XNOR gates
- **139 total gates**
- **gate depth 36**
- AND-depth 5

The release therefore removes one affine gate and one ordinary depth level while preserving the nonlinear gate count and AND-depth.

A separate exact computation also excludes a 23-product outer realization **inside the published 24-product outer span** used by the current 29-AND construction. This is a local exclusion only; it is not a global 29-AND lower bound.

## Status

- Public computational/engineering release
- Exhaustively verified on all 256 AES inputs
- External specialist review: pending
- NIST canonical adoption: pending

## Reproduce

The primary circuit check uses only Python's standard library:

```bash
python verify_candidate.py
```

Expected headline output:

```text
PASS
gates: 139 = 29 AND + 97 XOR + 13 XNOR
gate depth: 36
AND-depth: 5
AES truth table: 256/256 inputs matched
```

The local 28-AND frontier calculation can be replayed separately:

```bash
g++ -O3 -std=c++17 verify_28and_local_frontier.cpp -o verify_28and_local_frontier
./verify_28and_local_frontier
```

## Files

- `aes-sbox-fwd-a29-ad5-g139-gd36-xx110-13.ncff.txt` — optimized circuit
- `verify_candidate.py` — independent structural and exhaustive AES-equivalence verifier
- `verification-report.json` — machine-readable claim summary
- `verify_28and_local_frontier.cpp` — exact local-frontier verifier
- `verify_28and_local_frontier.out` — recorded replay output

## Baseline

The comparison baseline is pinned to the NIST Circuits repository at commit `4e23832e62f490aeffd8770b1285d99056b5f8bf`:

`data/slp/aes/sbox/aes-sbox-fwd-a29-ad5-g140-gd37-xx111-14.ncff.txt`

## Public release page

https://unsolved-labs.github.io/results/r002-aes-sbox/

## Claim boundary

This repository establishes the exact metrics and functional equivalence of the released circuit. It does not claim a production hardware, prover-time, MPC, FHE, or zero-knowledge performance improvement without implementation-specific benchmarking.
