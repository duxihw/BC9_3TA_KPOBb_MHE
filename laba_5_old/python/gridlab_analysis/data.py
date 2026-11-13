"""Загрузка CSV, которые пишет grid_bench, с проверкой структуры."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

import pandas as pd

EXPECTED_COLUMNS = {
    "copy_move.csv": ["impl", "n", "elements", "operation", "ns", "allocations",
                      "bytes", "element_copies"],
    "access.csv": ["dims", "n", "elements", "method", "order", "ns_per_element"],
    "construct.csv": ["impl", "dims", "d0", "d1", "d2", "elements", "allocations",
                      "bytes", "bytes_per_element", "construct_ns", "destroy_ns"],
}


@dataclass
class BenchData:
    copy_move: pd.DataFrame
    access: pd.DataFrame
    construct: pd.DataFrame


def _read(path: Path) -> pd.DataFrame:
    if not path.exists():
        raise FileNotFoundError(
            f"{path} не найден — сначала запустите бенчмарк: grid_bench --out {path.parent}")
    df = pd.read_csv(path)
    missing = [c for c in EXPECTED_COLUMNS[path.name] if c not in df.columns]
    if missing:
        raise ValueError(f"{path.name}: нет колонок {missing}")
    if df.empty:
        raise ValueError(f"{path.name}: файл пуст")
    return df


def load(data_dir: Path) -> BenchData:
    return BenchData(
        copy_move=_read(data_dir / "copy_move.csv"),
        access=_read(data_dir / "access.csv"),
        construct=_read(data_dir / "construct.csv"),
    )
