# /// script
# requires-python = ">=3.10"
# ///

"""
Benchmark latLngToCell, cellToLatLng, cellToBoundary on two git refs.

Checks out each ref, injects the benchmark C file if needed, builds,
runs N times, and reports the min. Restores the original branch on exit.

Usage:
    uv run bench.py [ref_a] [ref_b]

Defaults to master vs vec3d-core.
"""

import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
BUILD = os.path.join(ROOT, "build")
BENCH_SRC = "src/apps/benchmarks/benchmarkCoreApi.c"
BENCH_BIN = os.path.join(BUILD, "bin/benchmarkCoreApi")
CMAKE_ENTRY = "    add_h3_benchmark(benchmarkCoreApi src/apps/benchmarks/benchmarkCoreApi.c)"
N_RUNS = 10


def git(*args):
    return subprocess.run(
        ["git", *args], capture_output=True, text=True, cwd=ROOT
    )


def current_ref():
    r = git("rev-parse", "--abbrev-ref", "HEAD")
    branch = r.stdout.strip()
    r = git("rev-parse", "--short", "HEAD")
    return branch, r.stdout.strip()


def checkout(ref, bench_content):
    """Switch to ref, injecting benchmark file and CMakeLists entry."""
    # Clean before switching
    git("checkout", "--", ".")
    src = os.path.join(ROOT, BENCH_SRC)
    r = git("ls-files", BENCH_SRC)
    if not r.stdout.strip() and os.path.exists(src):
        os.remove(src)

    git("checkout", ref)

    # Inject benchmark file
    os.makedirs(os.path.dirname(src), exist_ok=True)
    with open(src, "w") as f:
        f.write(bench_content)

    # Inject CMakeLists entry
    cmake = os.path.join(ROOT, "CMakeLists.txt")
    txt = open(cmake).read()
    if "benchmarkCoreApi" not in txt:
        txt = txt.replace(
            "add_h3_benchmark(benchmarkH3Api",
            CMAKE_ENTRY + "\n    add_h3_benchmark(benchmarkH3Api",
        )
        open(cmake, "w").write(txt)


def build():
    os.makedirs(BUILD, exist_ok=True)
    r = subprocess.run(
        "cmake -DCMAKE_BUILD_TYPE=Release .. && make benchmarkCoreApi",
        shell=True, capture_output=True, text=True, cwd=BUILD,
    )
    if r.returncode != 0:
        print(f"  BUILD FAILED:\n{r.stderr[-300:]}")
        return False
    return True


def bench(n_runs):
    """Run benchmark n_runs times, return {name: min_us}."""
    best = {}
    for _ in range(n_runs):
        r = subprocess.run([BENCH_BIN], capture_output=True, text=True)
        for line in r.stdout.splitlines():
            m = re.match(r"(\w+):\s+([\d.]+)\s+us/call", line)
            if m:
                name, us = m.group(1), float(m.group(2))
                if name not in best or us < best[name]:
                    best[name] = us
    return best


def bench_ref(ref, bench_content):
    """Checkout ref, build, benchmark. Returns {name: min_us} or None."""
    print(f"\n{'='*50}")
    print(f"Benchmarking: {ref}")
    print(f"{'='*50}")

    checkout(ref, bench_content)
    branch, sha = current_ref()
    print(f"  branch: {branch}  commit: {sha}")

    if not build():
        return None

    print(f"  Running {N_RUNS} iterations...")
    results = bench(N_RUNS)
    for name, us in results.items():
        print(f"  {name}: {us:.4f} us/call")
    return results


def main():
    ref_a = sys.argv[1] if len(sys.argv) > 1 else "master"
    ref_b = sys.argv[2] if len(sys.argv) > 2 else "vec3d-core"

    original, _ = current_ref()
    src = os.path.join(ROOT, BENCH_SRC)
    if not os.path.exists(src):
        print(f"Error: {BENCH_SRC} not found.")
        return
    bench_content = open(src).read()

    results = {}
    try:
        for ref in [ref_a, ref_b]:
            r = bench_ref(ref, bench_content)
            if r:
                results[ref] = r
    finally:
        checkout(original, bench_content)
        print(f"\nRestored: {original}")

    if len(results) == 2:
        print(f"\n{'='*50}")
        print(f"Comparison (min of {N_RUNS} runs)")
        print(f"{'='*50}")
        print(f"{'Function':<20} {ref_a:>12} {ref_b:>12} {'Change':>10}")
        print(f"{'-'*20} {'-'*12} {'-'*12} {'-'*10}")
        for name in results[ref_a]:
            a = results[ref_a][name]
            b = results[ref_b].get(name)
            if b is not None:
                pct = (b - a) / a * 100
                sign = "+" if pct > 0 else ""
                print(f"{name:<20} {a:>10.4f}us {b:>10.4f}us {sign}{pct:>8.1f}%")


if __name__ == "__main__":
    main()
