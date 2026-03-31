# /// script
# requires-python = ">=3.10"
# ///

"""
Run benchmarkCoreApi multiple times on two git refs and compare.

Copies the benchmark C file and CMakeLists entry to a temp location
so they survive branch switches. Restores the original branch on exit.

Usage: edit BRANCH_A, BRANCH_B, and N_RUNS below, then run with:
    uv run bench.py
"""

import subprocess
import re
import os
import shutil
import tempfile

BRANCH_A = "master"
BRANCH_B = "vec3d-core"
N_RUNS = 10
BUILD_DIR = "build"
BENCH_BIN = "bin/benchmarkCoreApi"

ROOT = os.path.dirname(os.path.abspath(__file__))
BENCH_SRC_REL = "src/apps/benchmarks/benchmarkCoreApi.c"
CMAKE_LINE = "    add_h3_benchmark(benchmarkCoreApi src/apps/benchmarks/benchmarkCoreApi.c)"


def run(cmd, **kwargs):
    return subprocess.run(cmd, capture_output=True, text=True,
                          cwd=ROOT, **kwargs)


def parse_output(text):
    results = {}
    for line in text.strip().split("\n"):
        m = re.match(r"(\w+):\s+([\d.]+)\s+us/call", line)
        if m:
            results[m.group(1)] = float(m.group(2))
    return results


def run_bench(n_runs):
    all_results = {}
    bin_path = os.path.join(ROOT, BUILD_DIR, BENCH_BIN)
    for i in range(n_runs):
        r = subprocess.run([bin_path], capture_output=True, text=True)
        parsed = parse_output(r.stdout)
        for name, us in parsed.items():
            if name not in all_results or us < all_results[name]:
                all_results[name] = us
    return all_results


def setup_bench_on_branch(bench_content):
    """Ensure benchmark C file and CMakeLists entry exist on current checkout."""
    bench_path = os.path.join(ROOT, BENCH_SRC_REL)
    cmake_path = os.path.join(ROOT, "CMakeLists.txt")

    os.makedirs(os.path.dirname(bench_path), exist_ok=True)
    with open(bench_path, "w") as f:
        f.write(bench_content)

    with open(cmake_path) as f:
        cmake = f.read()
    if "benchmarkCoreApi" not in cmake:
        cmake = cmake.replace(
            "add_h3_benchmark(benchmarkH3Api",
            CMAKE_LINE + "\n    add_h3_benchmark(benchmarkH3Api"
        )
        with open(cmake_path, "w") as f:
            f.write(cmake)


def cleanup_branch():
    """Restore tracked files and remove untracked benchmark file."""
    bench_path = os.path.join(ROOT, BENCH_SRC_REL)
    run(["git", "checkout", "--", "."])
    # Remove benchmark file if it's untracked (doesn't exist on this branch)
    r = run(["git", "ls-files", BENCH_SRC_REL])
    if not r.stdout.strip() and os.path.exists(bench_path):
        os.remove(bench_path)


def main():
    # Save current branch
    r = run(["git", "rev-parse", "--abbrev-ref", "HEAD"])
    original_branch = r.stdout.strip()

    # Save benchmark C file content before any branch switches
    bench_path = os.path.join(ROOT, BENCH_SRC_REL)
    if not os.path.exists(bench_path):
        print(f"Error: {BENCH_SRC_REL} not found. Run from the branch that has it.")
        return
    with open(bench_path) as f:
        bench_content = f.read()

    results = {}
    try:
        for ref in [BRANCH_A, BRANCH_B]:
            print(f"\n{'='*50}")
            print(f"Benchmarking: {ref}")
            print(f"{'='*50}")

            run(["git", "checkout", ref])

            # Verify branch
            r = run(["git", "rev-parse", "--short", "HEAD"])
            sha = r.stdout.strip()
            r = run(["git", "rev-parse", "--abbrev-ref", "HEAD"])
            branch = r.stdout.strip()
            print(f"  branch: {branch}  commit: {sha}")

            setup_bench_on_branch(bench_content)

            # Build
            run(["mkdir", "-p", BUILD_DIR])
            r = subprocess.run(
                f"cd {BUILD_DIR} && cmake -DCMAKE_BUILD_TYPE=Release .. && make benchmarkCoreApi",
                shell=True, capture_output=True, text=True, cwd=ROOT
            )
            if r.returncode != 0:
                print(f"Build failed on {ref}:")
                print(r.stderr[-500:])
                continue

            print(f"Running {N_RUNS} iterations...")
            results[ref] = run_bench(N_RUNS)
            for name, us in results[ref].items():
                print(f"  {name}: {us:.4f} us/call (min of {N_RUNS})")

            cleanup_branch()
    finally:
        # Always restore original branch
        cleanup_branch()
        run(["git", "checkout", original_branch])
        print(f"\nRestored branch: {original_branch}")

    # Print comparison
    if len(results) == 2:
        print(f"\n{'='*50}")
        print(f"Comparison (min of {N_RUNS} runs)")
        print(f"{'='*50}")
        print(f"{'Function':<20} {BRANCH_A:>12} {BRANCH_B:>12} {'Change':>10}")
        print(f"{'-'*20} {'-'*12} {'-'*12} {'-'*10}")

        for name in results[BRANCH_A]:
            a = results[BRANCH_A][name]
            b = results[BRANCH_B].get(name)
            if b is not None:
                pct = (b - a) / a * 100
                sign = "+" if pct > 0 else ""
                print(f"{name:<20} {a:>10.4f}us {b:>10.4f}us {sign}{pct:>8.1f}%")


if __name__ == "__main__":
    main()
