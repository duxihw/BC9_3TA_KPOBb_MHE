import os
import sys

# Добавляем путь к папке tools, чтобы импортировать visualizer
sys.path.append(os.path.join('..', '..', 'tools'))

try:
    from visualizer import LabVisualizer
except ImportError:
    print("Ошибка: visualizer.py не найден в папке tools.")
    sys.exit(1)

def run_analysis():
    viz = LabVisualizer(lab_name="laba_2")

    # --- 1. Обработка Сортировки Шелла ---
    try:
        shell_df = viz.load_data("shell_experiment.csv")

        # График времени работы (сравнение последовательностей)
        viz.create_plot()
        viz.add_line(shell_df['N'], shell_df['ShellTime_ms'], label='Classic (N/2^k)', color='#3498db')
        viz.add_line(shell_df['N'], shell_df['HibbardTime_ms'], label='Hibbard (2^i - 1)', color='#e74c3c', marker='s')
        viz.add_line(shell_df['N'], shell_df['FiboTime_ms'], label='Fibonacci', color='#2ecc71', marker='^')
        
        viz.set_labels(
            title="Сравнение времени работы: Сортировка Шелла",
            xlabel="Количество элементов (N)",
            ylabel="Время (мс)"
        )
        viz.save_plot("shell_time.png")

        # График количества перестановок
        viz.create_plot()
        viz.add_line(shell_df['N'], shell_df['ShellSwaps'], label='Classic', color='#3498db')
        viz.add_line(shell_df['N'], shell_df['HibbardSwaps'], label='Hibbard', color='#e74c3c', marker='s')
        viz.add_line(shell_df['N'], shell_df['FiboSwaps'], label='Fibonacci', color='#2ecc71', marker='^')
        
        viz.set_labels(
            title="Сравнение перестановок: Сортировка Шелла",
            xlabel="Количество элементов (N)",
            ylabel="Число операций (Swaps)"
        )
        viz.save_plot("shell_swaps.png")

    except FileNotFoundError:
        print("Файл shell_experiment.csv не найден.")

    # --- 2. Обработка Сортировки Расчёской ---
    try:
        comb_df = viz.load_data("comb_experiment.csv")

        viz.create_plot()
        viz.add_line(comb_df['N'], comb_df['CombTime_ms'], label='Comb Sort', color='#9b59b6')
        
        viz.set_labels(
            title="Производительность сортировки расчёской (N)",
            xlabel="Количество элементов (N)",
            ylabel="Время (мс)"
        )
        viz.save_plot("comb_N.png")

        viz.create_plot()
        viz.add_line(comb_df['N'], comb_df['CombSteps'], label='Comb Sort', color='#9b59b6')
        
        viz.set_labels(
            title="Производительность сортировки расчёской (swaps)",
            xlabel="Количество перестановок (swaps)",
            ylabel="Время (мс)"
        )
        viz.save_plot("comb_swaps.png")

    except FileNotFoundError:
        print("Файл comb_experiment.csv не найден.")

    # --- 3. Обработка Шейкерной Сортировки ---
    try:
        comb_df = viz.load_data("shaker_experiment.csv")

        viz.create_plot()
        viz.add_line(comb_df['N'], comb_df['ForwardTime_ms'], label='Shaker Sort', color='#3498db')
        viz.add_line(comb_df['N'], comb_df['BackwardTime_ms'], label='Shaker Sort', color='#e74c3c')
        viz.add_line(comb_df['N'], comb_df['ShakerTime_ms'], label='Shaker Sort', color='#2ecc71')
        
        viz.set_labels(
            title="Производительность шейкерной сортировки",
            xlabel="Количество элементов (N)",
            ylabel="Время (мс)"
        )
        viz.save_plot("shaker_N.png")

    except FileNotFoundError:
        print("Файл shaker_experiment.csv не найден.")

if __name__ == "__main__":
    run_analysis()