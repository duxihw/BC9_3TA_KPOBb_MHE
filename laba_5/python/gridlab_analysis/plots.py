"""Графики (matplotlib -> PNG).

Оформление: тонкие линии, маркеры, светлая сетка; цвет закреплён за
реализацией (flat — синий, nested — оранжевый и т.д.), а не за порядком
серий; второстепенное различие (операция, порядок обхода) — стилем линии.
"""

from __future__ import annotations

from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
import numpy as np  # noqa: E402
import pandas as pd  # noqa: E402

BLUE, ORANGE, AQUA, YELLOW, MAGENTA = "#2a78d6", "#eb6834", "#1baf7a", "#eda100", "#e87ba4"
TEXT, MUTED, GRID = "#2b2b2b", "#6b6b6b", "#e4e4e0"

IMPL_COLORS = {"flat": BLUE, "nested": ORANGE}
IMPL_LABELS = {"flat": "flat — v1::Grid<T> (один блок)",
               "nested": "nested — Grid<T,N> (рекурсивная)"}
METHOD_COLORS = {
    "std::vector": MUTED,
    "flat g(y,x)": BLUE,
    "flat g[y][x]": AQUA,
    "nested g(y,x)": ORANGE,
    "nested g[y][x]": YELLOW,
    "nested g(z,y,x)": ORANGE,
    "nested g[z][y][x]": YELLOW,
}

plt.rcParams.update({
    "figure.dpi": 110, "savefig.dpi": 150, "font.size": 10,
    "axes.edgecolor": MUTED, "axes.labelcolor": TEXT, "axes.titlecolor": TEXT,
    "axes.titleweight": "bold", "axes.grid": True, "grid.color": GRID,
    "grid.linewidth": 0.8, "axes.spines.top": False, "axes.spines.right": False,
    "xtick.color": MUTED, "ytick.color": MUTED, "legend.frameon": False,
    "lines.linewidth": 2, "lines.markersize": 5,
})


def _save(fig: plt.Figure, out: Path) -> Path:
    fig.tight_layout()
    fig.savefig(out, bbox_inches="tight", facecolor="white")
    plt.close(fig)
    return out


# ------------------------------------------------------------------ copy / move
def copy_move_time(df: pd.DataFrame, out: Path) -> Path:
    """Время копирования и перемещения от числа элементов (log-log)."""
    fig, ax = plt.subplots(figsize=(9, 5.5))
    styles = {"copy_ctor": ("-", "o", "копирование"), "move_ctor": ("--", "s", "перемещение")}
    for impl, color in IMPL_COLORS.items():
        for op, (ls, mk, op_label) in styles.items():
            s = df[(df.impl == impl) & (df.operation == op)].sort_values("elements")
            ax.plot(s.elements, s.ns, ls, marker=mk, color=color,
                    label=f"{op_label}, {impl}")
    # Опорная прямая с наклоном 1: время ∝ числу элементов.
    e = np.array([df.elements.min(), df.elements.max()], dtype=float)
    copy = df[(df.impl == "flat") & (df.operation == "copy_ctor")].sort_values("elements")
    ref = copy.ns.iloc[-1] * e / e[-1]
    ax.plot(e, ref, ":", color=MUTED, linewidth=1.5, label="наклон 1: Θ(элементов)")
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("элементов в сетке (лог. шкала)")
    ax.set_ylabel("нс на операцию (лог. шкала)")
    ax.set_title("Копирование растёт линейно, перемещение — константа")
    ax.legend(ncol=2, fontsize=9)
    return _save(fig, out)


def copy_move_resources(df: pd.DataFrame, out: Path) -> Path:
    """Выделения памяти и копии элементов на одну операцию."""
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 4.8))
    for impl, color in IMPL_COLORS.items():
        s = df[(df.impl == impl) & (df.operation == "copy_ctor")].sort_values("n")
        ax1.plot(s.n, s.allocations, marker="o", color=color, label=IMPL_LABELS[impl])
        s2 = df[(df.impl == impl) & (df.operation == "move_ctor")].sort_values("n")
        ax2.plot(s.elements, s.element_copies, marker="o", color=color,
                 label=f"копирование, {impl}")
        ax2.plot(s2.elements, s2.element_copies + 0.5, "--", marker="s", color=color,
                 label=f"перемещение, {impl} (0, сдвинуто для лог. шкалы)")
    ax1.set_xscale("log", base=2)
    ax1.set_yscale("log")
    ax1.set_xlabel("n (сетка n × n)")
    ax1.set_ylabel("выделений памяти на копию")
    ax1.set_title("Выделения памяти при копировании")
    ax1.legend()
    ax2.set_xscale("log")
    ax2.set_yscale("log")
    ax2.set_xlabel("элементов в сетке")
    ax2.set_ylabel("копий элементов на операцию")
    ax2.set_title("Копии элементов (Tracked): линии flat и nested совпадают")
    ax2.legend(fontsize=8)
    return _save(fig, out)


# ------------------------------------------------------------------ доступ
def access(df: pd.DataFrame, dims: int, out: Path) -> Path:
    """Нс на элемент при обходе по строкам и по столбцам."""
    sub = df[df.dims == dims]
    fig, axes = plt.subplots(1, 2, figsize=(12, 4.8))
    titles = {"rows": "Обход по строкам (последний индекс — внутренний цикл)",
              "columns": "Обход по столбцам (первый индекс — внутренний цикл)"}
    for ax, order in zip(axes, ["rows", "columns"]):
        s = sub[sub.order == order]
        for method in s.method.unique():
            m = s[s.method == method].sort_values("n")
            ls = ":" if method == "std::vector" else "-"
            ax.plot(m.n, m.ns_per_element, ls, marker="o", color=METHOD_COLORS[method],
                    label=method)
        ax.set_xscale("log", base=2)
        ax.set_ylim(bottom=0)
        side = "n × n" if dims == 2 else "n × n × n"
        ax.set_xlabel(f"n (сетка {side})")
        ax.set_ylabel("нс на элемент")
        ax.set_title(titles[order], fontsize=10)
    axes[0].legend()
    fig.suptitle(f"Доступ к элементам, {dims}D (int32, сумма всех элементов)",
                 color=TEXT, fontsize=12)
    return _save(fig, out)


# ------------------------------------------------------------------ создание
def construct(df: pd.DataFrame, out: Path) -> Path:
    """Создание сеток одинакового объёма разной формы."""
    series = [
        ("flat", 2, BLUE, "flat 2D: d0 × (E/d0)"),
        ("nested", 2, ORANGE, "nested 2D: d0 × (E/d0)"),
        ("nested", 3, AQUA, "nested 3D: d0 × d0 × (E/d0²)"),
    ]
    fig, axes = plt.subplots(1, 3, figsize=(15, 4.6))
    total = int(df.elements.iloc[0])
    for impl, dims, color, label in series:
        s = df[(df.impl == impl) & (df.dims == dims)].sort_values("d0")
        axes[0].plot(s.d0, s.allocations, marker="o", color=color, label=label)
        axes[1].plot(s.d0, s.construct_ns / 1e3, marker="o", color=color, label=label)
        axes[2].plot(s.d0, s.bytes_per_element, marker="o", color=color, label=label)
    for ax in axes:
        ax.set_xscale("log", base=2)
        ax.set_xlabel("d0 — размер по первой оси")
    axes[0].set_yscale("log")
    axes[0].set_ylabel("выделений памяти")
    axes[0].set_title("Число выделений памяти")
    axes[1].set_yscale("log")
    axes[1].set_ylabel("мкс на создание")
    axes[1].set_title("Время создания")
    axes[2].axhline(4, color=MUTED, linestyle=":", linewidth=1)
    axes[2].text(1, 4.3, "sizeof(float) = 4", color=MUTED)
    axes[2].set_ylabel("байт на элемент")
    axes[2].set_title("Память на элемент")
    axes[0].legend()
    fig.suptitle(f"Сетки из E = {total:,} элементов float разной формы".replace(",", " "),
                 color=TEXT, fontsize=12)
    return _save(fig, out)
