#!/usr/bin/env python3
"""One-command clean-checkout verification for R002."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
CANDIDATE = HERE / "aes-sbox-fwd-a29-ad5-g139-gd36-xx110-13.ncff.txt"
EXPECTED_SHA256 = "1dc1d09133e90dd435f7d87b78a9e1413418ce10cf82507afd57432b5a562577"


def run(cmd: list[str]) -> str:
    print("+", " ".join(cmd), flush=True)
    completed = subprocess.run(cmd, cwd=HERE, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=True)
    print(completed.stdout, end="")
    return completed.stdout


def require(text: str, fragments: list[str]) -> None:
    for fragment in fragments:
        if fragment not in text:
            raise SystemExit(f"missing expected verifier output: {fragment!r}")


def main() -> None:
    got_sha = hashlib.sha256(CANDIDATE.read_bytes()).hexdigest()
    if got_sha != EXPECTED_SHA256:
        raise SystemExit(f"candidate SHA-256 mismatch: {got_sha}")

    manifest = json.loads((HERE / "artifact-manifest.json").read_text(encoding="utf-8"))
    for item in manifest["artifacts"]:
        artifact = HERE / item["path"]
        digest = hashlib.sha256(artifact.read_bytes()).hexdigest()
        if digest != item["sha256"]:
            raise SystemExit(f"artifact hash mismatch for {item['path']}: {digest}")

    report = json.loads((HERE / "verification-report.json").read_text(encoding="utf-8"))
    if report["candidate"]["sha256"] != got_sha:
        raise SystemExit("verification-report candidate SHA-256 mismatch")
    if report["candidate"]["metrics"] != {
        "and": 29, "xor": 97, "xnor": 13, "affine": 110,
        "gates": 139, "gate_depth": 36, "and_depth": 5,
        "and_depth_profile": [9, 1, 2, 2, 15],
    }:
        raise SystemExit("verification-report candidate metrics changed")

    primary = run(["python3", "verify_candidate.py"])
    require(primary, ["PASS primary Python verifier", "256/256 inputs matched"])

    rewrite = run(["python3", "verify_rewrite_identity.py"])
    require(rewrite, ["PASS affine rewrite identity", "assignments checked: 128/128"])

    with tempfile.TemporaryDirectory(prefix="r002-") as td:
        td = Path(td)
        independent_bin = td / "verify_candidate_independent"
        run(["g++", "-O2", "-std=c++17", "-Wall", "-Wextra", "verify_candidate_independent.cpp", "-o", str(independent_bin)])
        independent = run([str(independent_bin)])
        require(independent, ["PASS independent C++ verifier", "256/256 inputs matched"])

        frontier_bin = td / "verify_28and_local_frontier"
        run(["g++", "-O3", "-std=c++17", "-Wall", "-Wextra", "verify_28and_local_frontier.cpp", "-o", str(frontier_bin)])
        frontier_cpp = run([str(frontier_bin)])
        expected_frontier = (HERE / "verify_28and_local_frontier.out").read_text(encoding="utf-8")
        if frontier_cpp != expected_frontier:
            raise SystemExit("C++ frontier replay differs from frozen output")

    frontier_py = run(["python3", "verify_frontier_independent.py"])
    common = [
        "canonical decomposable bivectors: 2794155",
        "published outer-span rank: 24",
        "decomposable points in published span: 27",
        "target rank: 12",
        "target-containing hyperplanes checked: 4095",
        "maximum decomposable span rank: 21",
    ]
    require(frontier_cpp, common)
    require(frontier_py, common + ["PASS independent Python frontier replay"])

    print("PASS R002 release verification")


if __name__ == "__main__":
    main()
