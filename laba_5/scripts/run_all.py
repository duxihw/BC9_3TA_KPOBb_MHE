#!/usr/bin/env python3
"""Полный цикл одной командой (Linux / macOS / Windows + MSYS2):

    python scripts/run_all.py          # сборка, тесты, бенчмарк, анализ
    python scripts/run_all.py --quick  # быстрый бенчмарк (уменьшенные размеры) 

Требуется: CMake >= 3.20, Ninja, компилятор C++20, Python 3 + requirements.txt.
"""

import argparse
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def run(*cmd: str) -> None:
    print("\n$", " ".join(cmd), flush=True)
    subprocess.run(cmd, cwd=ROOT, check=True)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--quick", action="store_true", help="уменьшенный бенчмарк")
    ap.add_argument("--skip-bench", action="store_true", help="только сборка и тесты")
    args = ap.parse_args()

    run("cmake", "--preset", "release")
    run("cmake", "--build", "--preset", "release")
    run("ctest", "--preset", "release")
    if not args.skip_bench:
        exe = ROOT / "build" / "release" / "bin" / ("grid_bench.exe" if sys.platform == "win32" else "grid_bench")
        bench = [str(exe), "--out", str(ROOT.parent / "data" / "laba_5")]
        if args.quick:
            bench.append("--quick")
        run(*bench)
        run(sys.executable, str(ROOT / "python" / "analyze.py"))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
