"""Сводные таблицы."""

from __future__ import annotations

from pathlib import Path

import numpy as np
import pandas as pd


def power_exponent(x: np.ndarray, y: np.ndarray) -> float:
    """Показатель α в y ≈ c·x^α (наклон в log-log осях).

    α ≈ 1 — время пропорционально числу элементов, α ≈ 0 — не зависит от него.
    """
    alpha, _ = np.polyfit(np.log(x), np.log(y), 1)
    return float(alpha)


def copy_move_fits(df: pd.DataFrame, min_elements: int = 4096) -> pd.DataFrame:
    """Наклон log-log для каждой реализации и операции (на больших сетках,
    где постоянные накладные расходы уже не доминируют)."""
    rows = []
    for (impl, op), g in df.groupby(["impl", "operation"]):
        g = g[g.elements >= min_elements].sort_values("elements")
        biggest = g.iloc[-1]
        rows.append({
            "impl": impl,
            "operation": op,
            "alpha": power_exponent(g.elements.to_numpy(float), g.ns.to_numpy(float)),
            "ns_at_max": biggest.ns,
            "ns_per_element_at_max": biggest.ns / biggest.elements,
            "allocations_at_max": int(biggest.allocations),
            "element_copies_at_max": int(biggest.element_copies),
            "n_max": int(biggest.n),
        })
    return pd.DataFrame(rows)


def access_summary(df: pd.DataFrame) -> pd.DataFrame:
    """Для каждого (dims, n, order): нс/элемент и отношение к std::vector."""
    base = (df[df.method == "std::vector"]
            .set_index(["dims", "n", "order"]).ns_per_element.rename("vector_ns"))
    out = df.join(base, on=["dims", "n", "order"])
    out["ratio_to_vector"] = out.ns_per_element / out.vector_ns
    return out.drop(columns=["vector_ns"])


def construct_check(df: pd.DataFrame) -> pd.DataFrame:
    """Сравнение числа выделений памяти с формулой.

    flat:       1 выделение;
    nested 2D:  1 + d0 (массив подсеток + по массиву элементов на подсетку);
    nested 3D:  1 + d0 + d0·d1.
    """
    out = df.copy()
    expected = np.where(
        out.impl == "flat", 1,
        np.where(out.dims == 2, 1 + out.d0, 1 + out.d0 + out.d0 * out.d1))
    out["expected_allocations"] = expected
    out["formula_ok"] = out.allocations == out.expected_allocations
    return out


def to_markdown(df: pd.DataFrame, floatfmt: str = ".3g") -> str:
    def fmt(v):
        if isinstance(v, (float, np.floating)):
            return format(v, floatfmt)
        return str(v)

    head = "| " + " | ".join(map(str, df.columns)) + " |"
    sep = "|" + "|".join("---" for _ in df.columns) + "|"
    body = ["| " + " | ".join(fmt(v) for v in row) + " |" for row in df.itertuples(index=False)]
    return "\n".join([head, sep, *body])


def write(tables: dict[str, pd.DataFrame], out_dir: Path) -> list[Path]:
    paths = []
    for name, table in tables.items():
        path = out_dir / f"{name}.csv"
        table.to_csv(path, index=False, float_format="%.6g")
        paths.append(path)
    return paths
