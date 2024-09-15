import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

def plot_montecarlo_simu_df(strat_name):
    files = os.listdir("../strat_outputs/")
    end_values = []
    for f in files:
        if 'MonteCarloSimu_' in f and strat_name in f:
            df = pd.read_csv('../strat_outputs/'+f, usecols=['Value'], sep=';')
            end_values.append(df.Value.values[-1])
            plt.plot(df.Value)
    print(f'Modes on end values: 25% : {np.quantile(end_values, 0.25)} - 50% : {np.quantile(end_values, 0.5)} - 75% : {np.quantile(end_values, 0.75)}')
    plt.xlabel('Days')
    plt.ylabel('Portfolio Value')

    ax_hist = plt.gca().inset_axes([0.4, 0.5, 0.25, 0.35])  # Inset histogram
    ax_hist.hist(end_values, bins=30, color='green', alpha=0.7)
    ax_hist.set_title('End Value Distribution')
    ax_hist.set_xlabel('Final Value')
    ax_hist.set_ylabel('Frequency')
    plt.tight_layout()
    plt.show()

def plot_strategy_df(filepath):
    
    df = pd.read_csv(filepath, usecols=['Value'], sep=';')
    plt.plot(df.Value)
    plt.xlabel('Days')
    plt.ylabel('Portfolio Value')
    plt.show()
    
if __name__ == '__main__':
    stratname = 'DCA_SPGold'
    plot_montecarlo_simu_df(stratname)
    plot_strategy_df('../strat_outputs/DCA_SPGold_acc_2015_2024.csv')