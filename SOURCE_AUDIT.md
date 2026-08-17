# R002 source and provenance audit

## AES specification

The functional specification is the forward AES S-box from:

- National Institute of Standards and Technology, **FIPS 197: Advanced Encryption Standard (AES)**, updated 2023 edition.
- DOI: `10.6028/NIST.FIPS.197-upd1`.
- Public landing page: https://csrc.nist.gov/pubs/fips/197/final

`verify_candidate.py` derives the S-box algebraically using inversion in GF(2^8) with the AES reduction polynomial and the affine output transform. `verify_candidate_independent.cpp` instead compares against the canonical 256-byte FIPS S-box table, providing an intentionally different specification path.

## Frozen NIST circuit comparison

Repository: `https://github.com/usnistgov/Circuits`

Pinned commit:

`4e23832e62f490aeffd8770b1285d99056b5f8bf`

Pinned path:

`data/slp/aes/sbox/aes-sbox-fwd-a29-ad5-g140-gd37-xx111-14.ncff.txt`

Git blob SHA:

`9c9d08980679aad4efcd1a459aa8c2cb27cf7e5b`

The pinned file records:

- 29 AND;
- 97 XOR;
- 14 XNOR;
- 140 total gates;
- gate depth 37;
- AND-depth 5; and
- AND-depth profile `9 1 2 2 15`.

The NIST file also records that it follows from transformations of the public `umizame/S-box_29-AND` circuit. R002 does not make a priority claim over that broader line of work; its comparison is strictly to the immutable NIST file above.

## Released candidate identity

File:

`aes-sbox-fwd-a29-ad5-g139-gd36-xx110-13.ncff.txt`

SHA-256:

`1dc1d09133e90dd435f7d87b78a9e1413418ce10cf82507afd57432b5a562577`

The candidate retains the same 29-AND nonlinear structure and changes an affine fragment. The exact fragment identity is documented in the manuscript and exhaustively checked by `verify_rewrite_identity.py`.

## Local-frontier certificate provenance

`frontier_certificate.txt` contains 24 factor pairs in F2^12 and 12 target-coordinate rows in F2^24. The two frontier verifiers independently derive the 66-coordinate wedge representations and replay the entire finite hyperplane calculation.

The current release treats the extraction of these frozen generator/target rows from the research pipeline as a provenance input. That is explicitly part of the trust boundary; the repository does not represent the certificate rows as independently re-derived from the full AES circuit.

## Licensing note

At the pinned NIST Circuits commit, the repository root contains no top-level license file. This upgrade therefore does not guess or impose a repository-wide license for the derived circuit artifact. Any future licensing change should first establish the status of the upstream circuit data and contributions explicitly.
