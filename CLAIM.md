# R002 claim and scope

## Release claim

R002 establishes three exact, separately scoped statements.

### C1 — released AES S-box circuit

The file `aes-sbox-fwd-a29-ad5-g139-gd36-xx110-13.ncff.txt` is a strict straight-line Boolean circuit over `AND`, `XOR`, and `XNOR` with:

- 29 AND gates;
- 97 XOR gates;
- 13 XNOR gates;
- 139 total gates;
- gate depth 36;
- AND-depth 5; and
- AND-depth profile `9 1 2 2 15`.

On all 256 possible 8-bit inputs, its eight output bits equal the forward AES S-box specified by FIPS 197.

### C2 — improvement over the frozen NIST comparison point

Against the NIST Circuits file pinned in `SOURCE_AUDIT.md`, whose recorded metrics are 29 AND, 97 XOR, 14 XNOR, 140 total gates, gate depth 37, and AND-depth 5, the released circuit removes one XNOR gate and one ordinary depth level while preserving the nonlinear gate count and AND-depth.

The changed affine fragment is proved equivalent on its downstream interface `(t59,t65,t66)` by `verify_rewrite_identity.py`.

### C3 — local 23-product outer-span exclusion

For the frozen 24-generator outer span and 12-dimensional target subspace encoded in `frontier_certificate.txt`, every target-containing hyperplane has decomposable-point span rank at most 21. Consequently, no 23-product outer realization exists entirely inside that frozen 24-product span.

This is a **local** finite exclusion relative to the frozen certificate data. It is not a global multiplicative-complexity lower bound for the AES S-box.

## Explicit non-claims

R002 does not claim:

- that 139 is a globally minimum total gate count for the AES S-box;
- that 29 AND gates is the global multiplicative complexity of the AES S-box;
- that 28-AND circuits are globally impossible;
- that the local frontier certificate exhausts outer spans other than the one encoded here;
- a hardware area, timing, power, FHE, MPC, or zero-knowledge performance improvement without implementation-specific benchmarking; or
- external specialist review or NIST adoption.

## Review status

External specialist review: **pending**.
