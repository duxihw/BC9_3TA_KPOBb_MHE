import os
import sys
import pandas as pd
import numpy as np

# Добавляем путь к папке tools
sys.path.append(os.path.join('..', '..', 'tools'))

try:
    from visualizer import LabVisualizer
except ImportError:
    print("Ошибка: visualizer.py не найден в папке tools.")
    sys.exit(1)

def run_analysis():
    viz = LabVisualizer(lab_name="laba_3")
    
    try:
        df = viz.load_data("summation_results.csv")
    except FileNotFoundError:
        print("Файл summation_results.csv не найден в папке данных.")
        return

    # --- 1. Подготовка данных для таблицы ---
    # Шаг интегрирования h = (10 * sqrt(T)) / (N_points - 1)
    df['Step_h'] = (10 * np.sqrt(df['T'])) / (df['N_points'] - 1)
    
    # Сохранение таблицы в текстовый файл
    table_txt_path = os.path.join(viz.plots_dir, "table_results.txt")
    with open(table_txt_path, "w", encoding="utf-8") as f:
        f.write("ТАБЛИЦА РЕЗУЛЬТАТОВ (Научный формат, 7 знаков)\n")
        f.write("="*110 + "\n")
        for method in df['Method'].unique():
            f.write(f"\n--- Метод: {method} ---\n")
            m_df = df[df['Method'] == method][['T', 'Step_h', 'Calculated', 'Rel_Error_%']].copy()
            
            for col in ['T', 'Step_h', 'Calculated', 'Rel_Error_%']:
                m_df[col] = m_df[col].map(lambda x: f"{x:.7e}")
            
            m_df = m_df.rename(columns={'Calculated': 'Ответ', 'Rel_Error_%': 'Ошибка (%)'})
            f.write(m_df.to_string(index=False) + "\n")
        f.write("\n" + "="*110 + "\n")
    
    print(f"Текстовая таблица сохранена в {table_txt_path}")

    # Создание картинки с таблицей
    viz.create_plot()
    viz.ax.axis('off')
    
    cell_text = []
    col_labels = ['Метод', 'Температура (T)', 'Шаг (h)', 'Ответ', 'Ошибка (%)']
    
    for method in df['Method'].unique():
        m_df = df[df['Method'] == method][['T', 'Step_h', 'Calculated', 'Rel_Error_%']]
        for i, row in enumerate(m_df.values):
            method_label = method if i == 0 else ""
            cell_text.append([
                method_label, 
                f"{row[0]:.7e}", 
                f"{row[1]:.7e}", 
                f"{row[2]:.7e}", 
                f"{row[3]:.7e}"
            ])

    # Отрисовка таблицы
    table = viz.ax.table(cellText=cell_text, colLabels=col_labels, loc='center', cellLoc='center')
    table.auto_set_font_size(False)
    table.set_fontsize(7)
    table.scale(1.4, 2.0)
    
    viz.save_plot("table_results.png", dpi=900)

    # --- 2. Построение столбчатой диаграммы ---
    # Диаграмма для |v|
    subset = df[(df['Variable'] == '|v|') & (df['N_points'] == 350)].copy()
    
    # Создаем уникальную метку для оси X (так как N фиксировано, выводим только T)
    subset['Label'] = subset.apply(lambda r: f"T={r['T']}", axis=1)
    
    methods = subset['Method'].unique()
    labels = subset['Label'].unique()
    
    viz.create_plot()
    
    x = np.arange(len(labels))
    width = 0.15
    
    
    for i, method in enumerate(methods):
        method_data = subset[subset['Method'] == method]
        
        y_values = [method_data[method_data['Label'] == l]['Rel_Error_%'].values[0] 
                    if l in method_data['Label'].values else 0 for l in labels]
        
        viz.ax.bar(x + (i - len(methods)/2 + 0.5) * width, y_values, width, label=method)

    viz.set_labels(
        title="Сравнение относительной ошибки методов суммирования",
        xlabel="Параметры эксперимента (T, N = 350)",
        ylabel="Относительная ошибка (%)"
    )
    

    viz.ax.set_xticks(x)
    viz.ax.set_xticklabels(labels)
    
    viz.save_plot("error_comparison_bar_v.png")


    # Диаграмма для v^2

    subset = df[(df['Variable'] == 'v^2') & (df['N_points'] == 350)].copy()
    
    # Создаем уникальную метку для оси X (так как N фиксировано, выводим только T)
    subset['Label'] = subset.apply(lambda r: f"T={r['T']}", axis=1)
    
    methods = subset['Method'].unique()
    labels = subset['Label'].unique()
    
    viz.create_plot()
    
    x = np.arange(len(labels))
    width = 0.15
    
    
    for i, method in enumerate(methods):
        method_data = subset[subset['Method'] == method]
        
        y_values = [method_data[method_data['Label'] == l]['Rel_Error_%'].values[0] 
                    if l in method_data['Label'].values else 0 for l in labels]
        
        viz.ax.bar(x + (i - len(methods)/2 + 0.5) * width, y_values, width, label=method)

    viz.set_labels(
        title="Сравнение относительной ошибки методов суммирования",
        xlabel="Параметры эксперимента (T, N = 350)",
        ylabel="Относительная ошибка (%)"
    )
    

    viz.ax.set_xticks(x)
    viz.ax.set_xticklabels(labels)
    
    viz.save_plot("error_comparison_bar_v2.png")

if __name__ == "__main__":
    run_analysis()