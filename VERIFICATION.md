# R002 verification and trust boundary

## One-command verification

Requirements:

- Python 3.12 or compatible Python 3 with standard library only;
- a C++17 compiler (`g++` is used in CI).

From a clean checkout:

```bash
python3 verify_release.py
```

The command is network-free and performs every release check described below.

## V1 — primary whole-circuit verifier

```bash
python3 verify_candidate.py
```

This parser checks strict single assignment / use-before-definition, gate counts, ordinary depth, AND-depth, and the AND-depth profile. It evaluates all 256 inputs simultaneously as Python bitsets.

Its reference AES S-box is **not** a hard-coded table: it is derived by GF(2^8) multiplication/inversion and the FIPS 197 affine transform.

## V2 — independent whole-circuit verifier

```bash
g++ -O2 -std=c++17 -Wall -Wextra verify_candidate_independent.cpp -o /tmp/r002-independent
/tmp/r002-independent
```

This implementation has a separate C++ parser, scalar one-input-at-a-time evaluator, independently recomputed metrics/depths, and the canonical FIPS 197 256-byte S-box table as its reference specification.

The deliberately different reference path reduces the chance that one shared implementation error makes both checks pass.

## V3 — exact affine rewrite identity

```bash
python3 verify_rewrite_identity.py
```

This verifies the five-gate NIST fragment and the four-gate R002 replacement on all 128 assignments to the seven free upstream Boolean quantities needed by the identity. It proves equality of the downstream interface wires `t59`, `t65`, and `t66` under the upstream linear identities used by the rewrite.

## V4 — local frontier, C++ replay

```bash
g++ -O3 -std=c++17 -Wall -Wextra verify_28and_local_frontier.cpp -o /tmp/r002-frontier
/tmp/r002-frontier
```

The verifier reads `frontier_certificate.txt`, derives every generator bivector from its factor pair, proves that the frozen outer span has rank 24, enumerates every decomposable bivector of F2^12, and checks all target-containing hyperplanes.

The checked-in `verify_28and_local_frontier.out` is the frozen expected output.

## V5 — independent local-frontier replay

```bash
python3 verify_frontier_independent.py
```

This is a separately written Python implementation of the finite exterior-algebra / linear-algebra calculation. It independently obtains:

- 2,794,155 canonical decomposable bivectors;
- outer-span rank 24;
- 27 decomposable points in the frozen span;
- target rank 12;
- 4,095 target-containing hyperplanes; and
- maximum decomposable span rank 21.

The C++ and Python implementations share the frozen certificate data but not their elimination, enumeration, or rank code.

## Why this release does not currently use Lean

The load-bearing statements in C1 and C3 are finite and are exhaustively replayed by independent exact implementations with no floating-point arithmetic and no external solver. Adding a proof assistant would be useful only if it further reduced a meaningful trust boundary. The most important remaining non-mechanical trust item is the provenance of the frozen local-frontier certificate rows, not numerical correctness of the finite calculation.

Accordingly, this release currently prioritizes two independent exact whole-circuit checks, two independent exact frontier replays, and a separate exhaustive proof of the local affine rewrite. This is a modality-specific application of the same release-assurance principles used by R001.

## Trust boundary

The final result trusts:

1. standard Python/C++ integer and Boolean semantics and the compiler/interpreter;
2. the public FIPS 197 AES specification;
3. the immutable upstream NIST circuit identity recorded in `SOURCE_AUDIT.md` for the comparison metrics; and
4. for C3 only, the provenance of the frozen generator/target rows in `frontier_certificate.txt` as the intended local outer-span model.

Search or AI-generation procedures are **not** in the correctness oracle: the released candidate and certificate are checked from their final frozen artifacts.

## External review status

External specialist review is **pending**. Passing these checks establishes exact machine replay; it does not substitute for an independent domain-expert review of scope and provenance.
