# R002 statement audit

This file maps public claims to the human manuscript and the exact machine checks that support them.

| Claim | Public wording | Manuscript location | Machine evidence | Remaining trust boundary |
|---|---|---|---|---|
| C1 | Released circuit computes the forward AES S-box on all 256 inputs | Theorem 1; Sections 2 and 5 | `verify_candidate.py`; `verify_candidate_independent.cpp` | Correct interpretation of FIPS 197 and compiler/runtime correctness. The two verifiers use different parsers/evaluation styles and different AES specifications. |
| C1 | 139 gates = 29 AND + 97 XOR + 13 XNOR | Theorem 1; Section 2 | Both whole-circuit verifiers parse and recount all gates | NCFF gate semantics as documented in the release and upstream NIST file |
| C1 | gate depth 36; AND-depth 5; profile 9,1,2,2,15 | Theorem 1; Section 2 | Both whole-circuit verifiers recompute depths from dependencies | Definition of ordinary depth and AND-depth stated in the manuscript |
| C2 | one-gate / one-depth improvement over frozen NIST file | Proposition 2; Sections 3–4 | Candidate metrics are recomputed; `verify_rewrite_identity.py` proves the local fragment preserves `t59,t65,t66`; upstream metrics are pinned in `SOURCE_AUDIT.md` | The upstream comparison file is referenced by immutable commit/path/Git blob rather than vendored |
| C2 | five affine gates replaced by four | Proposition 2; Section 4 | `verify_rewrite_identity.py` checks all 128 assignments to the free upstream bits used by the identity | None beyond Boolean XOR/XNOR semantics |
| C3 | no 23-product outer realization inside the frozen 24-product span | Theorem 3; Section 6 | `verify_28and_local_frontier.cpp`; independent `verify_frontier_independent.py`; frozen replay output | Faithful provenance of the frozen generator/target rows in `frontier_certificate.txt`; the finite linear-algebra calculation itself is independently replayed |

## Public wording rule

Any README, release-page, citation, or announcement derived from this repository must preserve the local/global distinction in C3. In particular, neither C2 nor C3 may be shortened to “29 AND is optimal” or “28 AND is impossible.”
