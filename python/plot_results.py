#!/usr/bin/env python
# coding: utf-8
# Config

# Para executar: 
# python plot_results.py
# OU
# python3 plot_results.py
# OU
# chmod +x plot_results.py
# ./plot_results.py

# Certificar de que tem as bibliotecas abaixo instaladas
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

df = pd.read_csv('../saida.csv')
print(df.head())

melt = pd.melt(df, id_vars=['N','algoritmo'], value_vars=['comparacoes','trocas','tempo_us'])
sns.catplot(kind='point', data=melt, x='N', y='value', hue='algoritmo', col='variable', ci='sd', sharey=False)
plt.show()
plt.savefig('grafico.png', dpi=128)
