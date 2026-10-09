import os
import sys

sys.path.append(os.path.join('..', '..', 'tools'))

try:
    from visualizer import LabVisualizer
except ImportError:
    print("Ошибка: visualizer.py не найден в папке tools.")
    sys.exit(1)


HISTOGRAMS = [
    ("dice100_normal.csv", "Dice(100), обычный бросок", "dice100_normal.png"),
    ("dice100_penalty.csv", "Dice(100), бросок со штрафом", "dice100_penalty.png"),
    ("dice100_bonus.csv", "Dice(100), бросок с преимуществом", "dice100_bonus.png"),
    ("tdp_normal.csv", "ThreeDicePool(6,6,6), обычный бросок", "tdp_normal.png"),
    ("tdp_penalty.csv", "ThreeDicePool(6,6,6), бросок со штрафом", "tdp_penalty.png"),
    ("tdp_bonus.csv", "ThreeDicePool(6,6,6), бросок с преимуществом", "tdp_bonus.png"),
    ("doubledice_penalty.csv", "DoubleDice, часть PenaltyDice", "doubledice_penalty.png"),
    ("doubledice_bonus.csv", "DoubleDice, часть BonusDice", "doubledice_bonus.png"),
]


def run_analysis():
    viz = LabVisualizer(lab_name="laba_4")

    for csv_name, title, png_name in HISTOGRAMS:
        try:
            df = viz.load_data(csv_name)
        except FileNotFoundError:
            print(f"Файл {csv_name} не найден в папке данных.")
            continue

        viz.create_plot()
        viz.ax.bar(df["value"], df["probability"], width=0.8, label="вероятность")
        viz.set_labels(title=title, xlabel="значение", ylabel="вероятность")
        viz.save_plot(png_name)

    try:
        ev_df = viz.load_data("expected_values.csv")
    except FileNotFoundError:
        print("Файл expected_values.csv не найден в папке данных.")
        return

    table_txt_path = os.path.join(viz.plots_dir, "expected_values.txt")
    with open(table_txt_path, "w", encoding="utf-8") as f:
        f.write("МАТЕМАТИЧЕСКОЕ ОЖИДАНИЕ (оценка по выборке)\n")
        f.write("=" * 50 + "\n")
        f.write(ev_df.to_string(index=False) + "\n")
    print(f"Таблица мат. ожиданий сохранена в {table_txt_path}")


if __name__ == "__main__":
    run_analysis()
