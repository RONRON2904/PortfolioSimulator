import os
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
    plt.xlabel('Days')
    plt.ylabel('Portfolio Value')

    ax_hist = plt.gca().inset_axes([0.4, 0.5, 0.25, 0.35])  # Inset histogram
    ax_hist.hist(end_values, bins=30, color='green', alpha=0.7)
    ax_hist.set_title('End Value Distribution')
    ax_hist.set_xlabel('Final Value')
    ax_hist.set_ylabel('Frequency')
    plt.tight_layout()
    plt.show()
    
if __name__ == '__main__':
    stratname = 'DCA'
    plot_montecarlo_simu_df(stratname)