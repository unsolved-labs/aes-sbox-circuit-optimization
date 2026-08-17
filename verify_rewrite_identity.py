#!/usr/bin/env python3
"""Exact truth-table proof of the five-gate -> four-gate affine rewrite used in R002.

The proof uses only the upstream linear identities needed by the rewrite. It is
separate from both whole-circuit AES equivalence verifiers.
"""
from itertools import product


def xnor(a: int, b: int) -> int:
    return 1 ^ a ^ b


def main() -> None:
    checked = 0
    # The seven free bits cover all assignments consistent with the upstream
    # identities used below; t45 need not be expanded further for the rewrite.
    for U0, U6, t5, t42, t37, a14, t45 in product((0, 1), repeat=7):
        t7 = U0 ^ t5
        t9 = U6 ^ t5
        t43 = xnor(U0, t42)
        t46 = t9 ^ t43
        t47 = U6 ^ t45
        t48 = t7 ^ t47

        # Pinned NIST baseline fragment.
        b57 = t37 ^ t42
        b58 = b57 ^ a14
        b59 = b58 ^ t48
        b65 = t42 ^ b59
        b66 = xnor(t46, b59)

        # R002 four-gate replacement.
        c57 = t37 ^ a14
        c65 = t48 ^ c57
        c66 = t45 ^ c57
        c59 = t42 ^ c65

        assert (c59, c65, c66) == (b59, b65, b66)
        checked += 1

    assert checked == 128
    print("PASS affine rewrite identity")
    print("assignments checked: 128/128")
    print("preserved downstream wires: t59 t65 t66")
    print("baseline fragment: 5 affine gates; replacement: 4 affine gates")


if __name__ == "__main__":
    main()
