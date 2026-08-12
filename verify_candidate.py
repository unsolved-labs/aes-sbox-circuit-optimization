#!/usr/bin/env python3
"""Independent verifier for the Unsolved Labs 139-gate AES S-box circuit.

Checks:
- strict straight-line execution with no use-before-definition
- exact gate tally
- gate depth and AND-depth
- AND-depth profile
- exhaustive equality on all 256 inputs against an independently derived AES S-box
"""
from collections import Counter
from pathlib import Path

HERE = Path(__file__).resolve().parent
CIRCUIT = HERE / "aes-sbox-fwd-a29-ad5-g139-gd36-xx110-13.ncff.txt"
MASK256 = (1 << 256) - 1


def parse_ops(text: str):
    ops = []
    active = False
    for raw in text.splitlines():
        line = raw.strip()
        if line == "begin SLP":
            active = True
            continue
        if line == "end SLP":
            active = False
            continue
        if active and line and not line.startswith("%"):
            fields = line.split()
            if len(fields) != 4 or fields[0] not in {"XOR", "XNOR", "AND"}:
                raise SystemExit(f"bad gate line: {raw}")
            ops.append(tuple(fields))
    return ops


def gf_mul(a: int, b: int) -> int:
    out = 0
    for _ in range(8):
        if b & 1:
            out ^= a
        high = a & 0x80
        a = (a << 1) & 0xFF
        if high:
            a ^= 0x1B
        b >>= 1
    return out


def gf_pow(a: int, e: int) -> int:
    out = 1
    while e:
        if e & 1:
            out = gf_mul(out, a)
        a = gf_mul(a, a)
        e >>= 1
    return out


def rotl8(x: int, n: int) -> int:
    return ((x << n) | (x >> (8 - n))) & 0xFF


def aes_sbox(x: int) -> int:
    y = 0 if x == 0 else gf_pow(x, 254)
    return y ^ rotl8(y, 1) ^ rotl8(y, 2) ^ rotl8(y, 3) ^ rotl8(y, 4) ^ 0x63


def main() -> None:
    ops = parse_ops(CIRCUIT.read_text(encoding="utf-8"))
    counts = Counter(kind for kind, *_ in ops)
    assert len(ops) == 139, len(ops)
    assert counts == Counter({"XOR": 97, "AND": 29, "XNOR": 13}), counts

    values = {}
    depth = {}
    and_depth = {}
    for i in range(8):
        values[f"U{i}"] = sum(1 << x for x in range(256) if (x >> (7 - i)) & 1)
        depth[f"U{i}"] = 0
        and_depth[f"U{i}"] = 0

    assigned = set(values)
    for kind, out, left, right in ops:
        assert out not in assigned, f"duplicate assignment: {out}"
        assert left in values and right in values, f"use before definition at {out}: {left}, {right}"
        if kind == "XOR":
            values[out] = values[left] ^ values[right]
        elif kind == "XNOR":
            values[out] = MASK256 ^ (values[left] ^ values[right])
        else:
            values[out] = values[left] & values[right]
        depth[out] = max(depth[left], depth[right]) + 1
        and_depth[out] = max(and_depth[left], and_depth[right]) + (kind == "AND")
        assigned.add(out)

    gate_depth = max(depth[f"S{i}"] for i in range(8))
    multiplicative_depth = max(and_depth[f"S{i}"] for i in range(8))
    profile = Counter(and_depth[out] for kind, out, *_ in ops if kind == "AND")
    assert gate_depth == 36, gate_depth
    assert multiplicative_depth == 5, multiplicative_depth
    assert [profile[i] for i in range(1, 6)] == [9, 1, 2, 2, 15], profile

    for x in range(256):
        got = sum(((values[f"S{i}"] >> x) & 1) << (7 - i) for i in range(8))
        expected = aes_sbox(x)
        assert got == expected, f"AES mismatch at {x:02x}: {got:02x} != {expected:02x}"

    print("PASS")
    print("gates: 139 = 29 AND + 97 XOR + 13 XNOR")
    print("gate depth: 36")
    print("AND-depth: 5")
    print("AND-depth profile: 9 1 2 2 15")
    print("AES truth table: 256/256 inputs matched")


if __name__ == "__main__":
    main()
