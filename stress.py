# /// script
# requires-python = ">=3.10"
# ///

"""
Run dumpCoreApi on two git refs and compare results.

Usage: edit BRANCH_A, BRANCH_B below, then run with:
    uv run stress.py
"""

import subprocess
import os
import math

BRANCH_A = "master"
BRANCH_B = "vec3d-core"
BUILD_DIR = "build"
DUMP_BIN = "bin/dumpCoreApi"

ROOT = os.path.dirname(os.path.abspath(__file__))
DUMP_SRC_REL = "src/apps/benchmarks/dumpCoreApi.c"
CMAKE_LINE = "    add_h3_benchmark(dumpCoreApi src/apps/benchmarks/dumpCoreApi.c)"


def run(cmd):
    return subprocess.run(cmd, capture_output=True, text=True, cwd=ROOT)


def prepare_branch(dump_content):
    """Write dump file and CMakeLists entry onto current checkout."""
    dump_path = os.path.join(ROOT, DUMP_SRC_REL)
    cmake_path = os.path.join(ROOT, "CMakeLists.txt")

    os.makedirs(os.path.dirname(dump_path), exist_ok=True)
    with open(dump_path, "w") as f:
        f.write(dump_content)

    with open(cmake_path) as f:
        cmake = f.read()
    if "dumpCoreApi" not in cmake:
        cmake = cmake.replace(
            "add_h3_benchmark(benchmarkH3Api",
            CMAKE_LINE + "\n    add_h3_benchmark(benchmarkH3Api"
        )
        with open(cmake_path, "w") as f:
            f.write(cmake)


def restore_branch():
    """Undo modifications to tracked files, remove untracked dump file."""
    run(["git", "checkout", "--", "."])
    dump_path = os.path.join(ROOT, DUMP_SRC_REL)
    # Remove if untracked on this branch
    r = run(["git", "ls-files", DUMP_SRC_REL])
    if not r.stdout.strip() and os.path.exists(dump_path):
        os.remove(dump_path)


def switch_to(ref):
    """Clean up current branch and switch to ref."""
    restore_branch()
    r = run(["git", "checkout", ref])
    if r.returncode != 0:
        print(f"  checkout failed: {r.stderr.strip()}")
        return False
    return True


def get_branch_info():
    r1 = run(["git", "rev-parse", "--abbrev-ref", "HEAD"])
    r2 = run(["git", "rev-parse", "--short", "HEAD"])
    return r1.stdout.strip(), r2.stdout.strip()


def build_and_run(ref, dump_content):
    print(f"\n{'='*50}")
    print(f"Running: {ref}")
    print(f"{'='*50}")

    if not switch_to(ref):
        return None

    branch, sha = get_branch_info()
    print(f"  branch: {branch}  commit: {sha}")

    prepare_branch(dump_content)

    r = subprocess.run(
        f"cd {BUILD_DIR} && cmake -DCMAKE_BUILD_TYPE=Release .. && make dumpCoreApi",
        shell=True, capture_output=True, text=True, cwd=ROOT
    )
    if r.returncode != 0:
        print(f"Build failed: {r.stderr[-300:]}")
        return None

    bin_path = os.path.join(ROOT, BUILD_DIR, DUMP_BIN)
    r = subprocess.run([bin_path], capture_output=True, text=True)
    print(f"  {len(r.stdout.splitlines())} lines of output")
    return r.stdout


def parse_sections(text):
    sections = {}
    current = None
    for line in text.splitlines():
        if line.startswith("#section "):
            current = line.split(" ", 1)[1]
            sections[current] = []
        elif current is not None:
            sections[current].append(line)
    return sections


def compare_latLngToCell(lines_a, lines_b, label="latLngToCell"):
    print(f"\n--- {label} ---")
    if len(lines_a) != len(lines_b):
        print(f"  LINE COUNT MISMATCH: {len(lines_a)} vs {len(lines_b)}")
        return

    mismatches = 0
    for la, lb in zip(lines_a, lines_b):
        pa = la.split()
        pb = lb.split()
        if pa[3] != pb[3]:
            mismatches += 1
            if mismatches <= 20:
                print(f"  MISMATCH: lat={pa[0]} lng={pa[1]} res={pa[2]}")
                print(f"    {BRANCH_A}: {pa[3]}")
                print(f"    {BRANCH_B}: {pb[3]}")

    print(f"  Total: {len(lines_a)}")
    print(f"  Mismatches: {mismatches}")
    if mismatches > 20:
        print(f"  (showing first 20 of {mismatches})")


def compare_cellToLatLng(lines_a, lines_b):
    print(f"\n--- cellToLatLng ---")
    if len(lines_a) != len(lines_b):
        print(f"  LINE COUNT MISMATCH: {len(lines_a)} vs {len(lines_b)}")
        return

    max_lat_diff = 0.0
    max_lng_diff = 0.0
    max_lat_cell = ""
    max_lng_cell = ""
    n_exact = 0

    for la, lb in zip(lines_a, lines_b):
        pa = la.split()
        pb = lb.split()
        lat_a, lng_a = float(pa[1]), float(pa[2])
        lat_b, lng_b = float(pb[1]), float(pb[2])

        dlat = abs(lat_a - lat_b)
        dlng = abs(lng_a - lng_b)
        if dlng > math.pi:
            dlng = 2 * math.pi - dlng

        if dlat == 0.0 and dlng == 0.0:
            n_exact += 1
        if dlat > max_lat_diff:
            max_lat_diff = dlat
            max_lat_cell = pa[0]
        if dlng > max_lng_diff:
            max_lng_diff = dlng
            max_lng_cell = pa[0]

    total = len(lines_a)
    print(f"  Total cells: {total}")
    print(f"  Exact matches: {n_exact} ({100*n_exact/total:.1f}%)")
    print(f"  Max lat diff: {max_lat_diff:.3e} rad  (cell {max_lat_cell})")
    print(f"  Max lng diff: {max_lng_diff:.3e} rad  (cell {max_lng_cell})")
    if max_lat_diff > 0:
        print(f"  Max lat diff: ~{max_lat_diff * 6.371e6:.6f} meters")
    if max_lng_diff > 0:
        print(f"  Max lng diff: ~{max_lng_diff * 6.371e6:.6f} meters")


def compare_cellToBoundary(lines_a, lines_b):
    print(f"\n--- cellToBoundary ---")
    if len(lines_a) != len(lines_b):
        print(f"  LINE COUNT MISMATCH: {len(lines_a)} vs {len(lines_b)}")
        return

    max_vert_diff = 0.0
    max_vert_cell = ""
    max_vert_idx = -1
    vert_count_mismatches = 0
    n_exact = 0

    for la, lb in zip(lines_a, lines_b):
        pa = la.split()
        pb = lb.split()
        nverts_a, nverts_b = int(pa[1]), int(pb[1])

        if nverts_a != nverts_b:
            vert_count_mismatches += 1
            continue

        cell_exact = True
        for v in range(nverts_a):
            lat_a = float(pa[2 + v * 2])
            lng_a = float(pa[3 + v * 2])
            lat_b = float(pb[2 + v * 2])
            lng_b = float(pb[3 + v * 2])

            dlat = abs(lat_a - lat_b)
            dlng = abs(lng_a - lng_b)
            if dlng > math.pi:
                dlng = 2 * math.pi - dlng

            diff = max(dlat, dlng)
            if diff > 0:
                cell_exact = False
            if diff > max_vert_diff:
                max_vert_diff = diff
                max_vert_cell = pa[0]
                max_vert_idx = v

        if cell_exact:
            n_exact += 1

    total = len(lines_a)
    print(f"  Total cells: {total}")
    print(f"  Exact matches: {n_exact} ({100*n_exact/total:.1f}%)")
    print(f"  Vertex count mismatches: {vert_count_mismatches}")
    print(f"  Max vertex diff: {max_vert_diff:.3e} rad  (cell {max_vert_cell}, vertex {max_vert_idx})")
    if max_vert_diff > 0:
        print(f"  Max vertex diff: ~{max_vert_diff * 6.371e6:.6f} meters")


def main():
    r = run(["git", "rev-parse", "--abbrev-ref", "HEAD"])
    original_branch = r.stdout.strip()

    # Read dump file content before any branch switching
    dump_path = os.path.join(ROOT, DUMP_SRC_REL)
    if not os.path.exists(dump_path):
        print(f"Error: {DUMP_SRC_REL} not found.")
        return
    with open(dump_path) as f:
        dump_content = f.read()

    outputs = {}
    try:
        for ref in [BRANCH_A, BRANCH_B]:
            output = build_and_run(ref, dump_content)
            if output is None:
                return
            outputs[ref] = output
    finally:
        switch_to(original_branch)
        print(f"\nRestored branch: {original_branch}")

    if len(outputs) != 2:
        return

    sec_a = parse_sections(outputs[BRANCH_A])
    sec_b = parse_sections(outputs[BRANCH_B])

    print(f"\n{'='*50}")
    print(f"Comparison: {BRANCH_A} vs {BRANCH_B}")
    print(f"{'='*50}")

    compare_latLngToCell(sec_a["latLngToCell"], sec_b["latLngToCell"])
    compare_latLngToCell(sec_a["latLngToCellVertices"], sec_b["latLngToCellVertices"],
                         label="latLngToCell (vertices & edges)")
    compare_cellToLatLng(sec_a["cellToLatLng"], sec_b["cellToLatLng"])
    compare_cellToBoundary(sec_a["cellToBoundary"], sec_b["cellToBoundary"])


if __name__ == "__main__":
    main()
