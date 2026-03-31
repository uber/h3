# /// script
# requires-python = ">=3.10"
# ///

"""
Run benchmarkCoreApi multiple times on two git refs and compare.

Usage: edit BRANCH_A, BRANCH_B, and N_RUNS below, then run with:
    uv run bench.py
"""

import subprocess
import re
import os

BRANCH_A = "master"
BRANCH_B = "vec3d-core"
N_RUNS = 10
BUILD_DIR = "build"
BENCH_BIN = "bin/benchmarkCoreApi"

ROOT = os.path.dirname(os.path.abspath(__file__))


def run(cmd, **kwargs):
    return subprocess.run(cmd, capture_output=True, text=True,
                          cwd=ROOT, **kwargs)


def build_bench(ref):
    """Check out ref, build benchmarkCoreApi in release mode."""
    run(["git", "checkout", ref])
    # The benchmark file may not exist on the other branch, so copy it
    run(["mkdir", "-p", f"{BUILD_DIR}"])
    r = subprocess.run(
        f"cd {BUILD_DIR} && cmake -DCMAKE_BUILD_TYPE=Release .. && make benchmarkCoreApi",
        shell=True, capture_output=True, text=True, cwd=ROOT
    )
    if r.returncode != 0:
        print(f"Build failed on {ref}:")
        print(r.stderr[-500:] if len(r.stderr) > 500 else r.stderr)
        return False
    return True


def parse_output(text):
    """Parse benchmark output lines into dict of {name: us_per_call}."""
    results = {}
    for line in text.strip().split("\n"):
        m = re.match(r"(\w+):\s+([\d.]+)\s+us/call", line)
        if m:
            results[m.group(1)] = float(m.group(2))
    return results


def run_bench(n_runs):
    """Run benchmark n_runs times, return dict of {name: min_us}."""
    all_results = {}
    bin_path = os.path.join(ROOT, BUILD_DIR, BENCH_BIN)
    for i in range(n_runs):
        r = subprocess.run([bin_path], capture_output=True, text=True)
        parsed = parse_output(r.stdout)
        for name, us in parsed.items():
            if name not in all_results or us < all_results[name]:
                all_results[name] = us
    return all_results


def main():
    # Save current branch
    r = run(["git", "rev-parse", "--abbrev-ref", "HEAD"])
    original_branch = r.stdout.strip()

    # Copy benchmark file so it exists on both branches
    bench_src = os.path.join(ROOT, "src/apps/benchmarks/benchmarkCoreApi.c")
    with open(bench_src) as f:
        bench_content = f.read()

    # Also need the CMakeLists line - read current CMakeLists
    cmake_path = os.path.join(ROOT, "CMakeLists.txt")
    with open(cmake_path) as f:
        cmake_content = f.read()
    cmake_line = "    add_h3_benchmark(benchmarkCoreApi src/apps/benchmarks/benchmarkCoreApi.c)"

    results = {}
    for ref in [BRANCH_A, BRANCH_B]:
        print(f"\n{'='*50}")
        print(f"Benchmarking: {ref}")
        print(f"{'='*50}")

        run(["git", "checkout", ref])

        # Ensure benchmark file exists on this branch
        with open(bench_src, "w") as f:
            f.write(bench_content)

        # Ensure CMakeLists has the benchmark entry
        with open(cmake_path) as f:
            current_cmake = f.read()
        if "benchmarkCoreApi" not in current_cmake:
            current_cmake = current_cmake.replace(
                "add_h3_benchmark(benchmarkH3Api",
                cmake_line + "\n    add_h3_benchmark(benchmarkH3Api"
            )
            with open(cmake_path, "w") as f:
                f.write(current_cmake)

        if not build_bench(ref):
            print(f"Skipping {ref}")
            continue

        print(f"Running {N_RUNS} iterations...")
        results[ref] = run_bench(N_RUNS)
        for name, us in results[ref].items():
            print(f"  {name}: {us:.4f} us/call (min of {N_RUNS})")

        # Clean up uncommitted benchmark file on non-original branches
        run(["git", "checkout", "--", "."])

    # Restore original branch
    run(["git", "checkout", original_branch])

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
