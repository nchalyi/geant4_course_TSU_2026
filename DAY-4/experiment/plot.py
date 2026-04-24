import numpy as np
import matplotlib.pyplot as plt

# ------------------------------------------------------------
# 1. Загрузка экспериментальных данных (exp_Meroli_...)
# ------------------------------------------------------------
exp_data = np.loadtxt('exp_Meroli_100MeV_electron_5p6um_Si.txt', 
                      comments='#')
exp_x = exp_data[:, 0]       # keV
exp_y = exp_data[:, 1]       # arb.units

# ------------------------------------------------------------
# 2. Загрузка и масштабирование моделируемых данных (Hist_Edep)
# ------------------------------------------------------------
# Предполагается формат: index, x, y
sim_data = np.loadtxt('Hist_Edep.txt')
sim_x_raw = sim_data[:, 1]   # x
sim_y_raw = sim_data[:, 2]   # y

# Масштабирование: x * 1000, y * 6.4
sim_x = sim_x_raw * 1000.0 
sim_y = sim_y_raw * 6.4

# ------------------------------------------------------------
# 3. Построение графика
# ------------------------------------------------------------
plt.figure(figsize=(9, 6))

# Экспериментальные данные — точки
plt.plot(exp_x, exp_y, 'o', markersize=3, color='black', 
         label='Эксперимент (Meroli et al.)')

# Моделируемые данные — кривая (соединяем линией)
plt.plot(sim_x, sim_y, '-', linewidth=1.8, color='red', 
         label='Моделирование (Hist_Edep, масштаб: x*1000, y*6.4)')

plt.xlabel('Energy loss, keV', fontsize=12)
plt.ylabel('Distribution, arb. units', fontsize=12)
plt.title('Сравнение экспериментального и моделированного распределений\n'
          '100 MeV electrons, 5.6 μm Si', fontsize=14)
plt.legend()
plt.grid(True, alpha=0.3)
plt.tight_layout()

# Показать график
plt.show()