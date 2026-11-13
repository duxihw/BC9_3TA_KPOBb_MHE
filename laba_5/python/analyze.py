#!/usr/bin/env python3
"""Анализ данных бенчмарка сетки Grid.

Читает ../data/laba_5/{copy_move,access,construct}.csv, строит графики в
../plots/laba_5/ и сохраняет сводные таблицы в ../data/laba_5/
(fit_copy_move.csv, summary_access.csv, check_construct.csv).

    python python/analyze.py [--data ../data/laba_5] [--plots ../plots/laba_5]
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from gridlab_analysis import data, plots, report  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--data", type=Path, default=ROOT.parent / "data" / "laba_5")
    parser.add_argument("--plots", type=Path, default=ROOT.parent / "plots" / "laba_5")
    args = parser.parse_args()
    args.plots.mkdir(parents=True, exist_ok=True)

    d = data.load(args.data)

    made = [
        plots.copy_move_time(d.copy_move, args.plots / "copy_move_time.png"),
        plots.copy_move_resources(d.copy_move, args.plots / "copy_move_resources.png"),
        plots.access(d.access, 2, args.plots / "access_2d.png"),
        plots.access(d.access, 3, args.plots / "access_3d.png"),
        plots.construct(d.construct, args.plots / "construct.png"),
    ]

    fits = report.copy_move_fits(d.copy_move)
    access = report.access_summary(d.access)
    check = report.construct_check(d.construct)
    written = report.write({"fit_copy_move": fits, "summary_access": access,
                            "check_construct": check}, args.data)

    print("Копирование/перемещение: показатель α в t ~ (элементов)^α")
    print(report.to_markdown(fits))

    print("\nДоступ на самых больших сетках (нс/элемент, отношение к std::vector):")
    for dims in sorted(access.dims.unique()):
        sub = access[access.dims == dims]
        top = sub[sub.n == sub.n.max()][["dims", "n", "method", "order",
                                         "ns_per_element", "ratio_to_vector"]]
        print(report.to_markdown(top))

    print("\nПроверка формулы числа выделений:",
          "все совпали" if check.formula_ok.all() else "ЕСТЬ РАСХОЖДЕНИЯ")
    print(report.to_markdown(check[["impl", "dims", "d0", "d1", "d2", "allocations",
                                    "expected_allocations", "bytes_per_element",
                                    "construct_ns"]]))

    print("\nГрафики:", *[f"  {p}" for p in made], sep="\n")
    print("Таблицы:", *[f"  {p}" for p in written], sep="\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
