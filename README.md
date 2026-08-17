# R002 — AES S-box circuit optimization

**Unsolved Labs Research Release R002**

R002 releases a forward AES S-box circuit with **139 Boolean gates and gate depth 36**, together with exact, independently replayable verification. The circuit preserves the 29-AND nonlinear structure of a frozen NIST comparison circuit while reducing the affine part by one gate and one ordinary depth level.

Research artifact generated with frontier AI and released by Unsolved Labs. External specialist review is **pending**.

## Result

| Metric | Frozen NIST comparison | R002 candidate |
|---|---:|---:|
| AND | 29 | 29 |
| XOR | 97 | 97 |
| XNOR | 14 | **13** |
| Total gates | 140 | **139** |
| Gate depth | 37 | **36** |
| AND-depth | 5 | 5 |
| AND-depth profile | 9, 1, 2, 2, 15 | 9, 1, 2, 2, 15 |

Both whole-circuit verifiers exhaustively check all 256 AES inputs.

The release also proves a separate **local** statement: inside the frozen 24-product outer span encoded in `frontier_certificate.txt`, no 23-product outer realization contains the frozen target subspace. This is not a global 28/29-AND lower bound.

## Read the proof

- [Technical manuscript (GitHub-renderable)](manuscript/r002-aes-sbox.md)
- [LaTeX source](manuscript/r002-aes-sbox.tex) — CI builds and uploads the PDF artifact
- [Exact claim and non-claims](CLAIM.md)
- [Statement-to-verification crosswalk](STATEMENT_AUDIT.md)
- [Verification and trust boundary](VERIFICATION.md)
- [Source/provenance audit](SOURCE_AUDIT.md)

## Reproduce everything

Requirements: Python 3 and a C++17 compiler.

```bash
python3 verify_release.py
```

This network-free command checks the candidate SHA-256, runs two independent full AES equivalence/metric verifiers, proves the affine rewrite identity, replays the local-frontier calculation in C++, and independently replays it in Python.

Expected final line:

```text
PASS R002 release verification
```

## Verification architecture

1. `verify_candidate.py` — Python bit-sliced evaluator; derives the AES S-box algebraically from FIPS 197 field arithmetic.
2. `verify_candidate_independent.cpp` — separate C++ parser/evaluator; compares all 256 outputs with the canonical FIPS 197 S-box table.
3. `verify_rewrite_identity.py` — exhaustive Boolean proof that the five-gate upstream fragment and four-gate replacement preserve `t59`, `t65`, and `t66`.
4. `verify_28and_local_frontier.cpp` — exact C++ replay of the frozen local exterior-algebra certificate.
5. `verify_frontier_independent.py` — separately implemented Python replay of the same finite local-frontier statement.

See `VERIFICATION.md` for the precise trust boundary and why this finite release currently uses independent exact replay rather than a Lean formalization.

## Frozen comparison source

The NIST comparison is pinned to commit:

`4e23832e62f490aeffd8770b1285d99056b5f8bf`

at:

`data/slp/aes/sbox/aes-sbox-fwd-a29-ad5-g140-gd37-xx111-14.ncff.txt`

Git blob SHA:

`9c9d08980679aad4efcd1a459aa8c2cb27cf7e5b`

Full provenance and scope are in `SOURCE_AUDIT.md`.

## Repository map

- `aes-sbox-fwd-a29-ad5-g139-gd36-xx110-13.ncff.txt` — released circuit.
- `frontier_certificate.txt` — frozen data for the local 23-product outer-span exclusion.
- `verification-report.json` / `claim.json` — machine-readable release metadata.
- `artifact-manifest.json` — frozen artifact hashes.
- `manuscript/` — GitHub-readable paper, LaTeX source, deterministic PDF build, and build instructions.
- `.github/workflows/verify.yml` — clean-checkout verification and manuscript build.

## Scope

R002 does **not** claim global optimality of 139 gates, a global 29-AND lower bound, or a universal implementation-level hardware/FHE/MPC/ZK advantage. See `CLAIM.md` for the complete boundary.

Public release page: https://unsolved-labs.github.io/results/r002-aes-sbox/
