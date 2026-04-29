import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

class LabVisualizer:
    def __init__(self, lab_name, project_root=os.path.join('..', '..')):
        """
        Инициализация визуализатора для конкретной лабораторной.
        project_root по умолчанию "..\\..", так как предполагается запуск скриптов из папок laba_X\\bin.
        """
        self.lab_name = lab_name
        self.data_dir = os.path.join(project_root, 'data', lab_name)
        self.plots_dir = os.path.join(project_root, 'plots', lab_name)

        # Создаем папку для графиков, если ее еще нет
        os.makedirs(self.plots_dir, exist_ok=True)

        self._apply_global_style()
        self.fig = None
        self.ax = None

    def _apply_global_style(self):
        """Единый конфиг внешнего вида для всех лабораторных."""
        plt.style.use('ggplot') # Базовый приятный стиль
        plt.rcParams['figure.figsize'] = (10, 6)
        plt.rcParams['axes.titlesize'] = 16
        plt.rcParams['axes.labelsize'] = 14
        plt.rcParams['xtick.labelsize'] = 12
        plt.rcParams['ytick.labelsize'] = 12
        plt.rcParams['lines.linewidth'] = 2.5
        plt.rcParams['lines.markersize'] = 8

    def load_data(self, filename):
        """Загрузка CSV данных с помощью pandas."""
        filepath = os.path.join(self.data_dir, filename)
        if not os.path.exists(filepath):
            raise FileNotFoundError(f"Файл с данными не найден: {filepath}")
        return pd.read_csv(filepath)

    def create_plot(self):
        """Создает новый холст для графика."""
        self.fig, self.ax = plt.subplots()

    def add_line(self, x, y, label=None, color=None, marker='o', linestyle='-'):
        """Добавляет линию на текущий график."""
        if self.ax is None:
            self.create_plot()
        self.ax.plot(x, y, label=label, color=color, marker=marker, linestyle=linestyle)

    def set_labels(self, title, xlabel, ylabel):
        """Настраивает заголовки и оси."""
        if self.ax is None:
            return
        self.ax.set_title(title, pad=15)
        self.ax.set_xlabel(xlabel, labelpad=10)
        self.ax.set_ylabel(ylabel, labelpad=10)
        self.ax.legend(loc='best', frameon=True, shadow=True)
        self.ax.grid(True, linestyle='--', alpha=0.7)

    def save_plot(self, filename, dpi=300):
        """Сохраняет график в папку plots/laba_X/."""
        if self.fig is None:
            print("Нет графика для сохранения. Сначала используйте create_plot().")
            return
        
        filepath = os.path.join(self.plots_dir, filename)
        self.fig.tight_layout()
        self.fig.savefig(filepath, dpi=dpi, bbox_inches='tight')
        plt.close(self.fig)
        
        self.fig = None
        self.ax = None
        print(f"График успешно сохранен: {filepath}")