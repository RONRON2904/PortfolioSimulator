#### WIP : This project is not finished yet but can be executed to backtest any strategy coded by any developer.

## Project Description
This C++ code is a high performance portfolio simulator / backtestor.    
The first idea behind this project was to mix my strong interests in **finance**, **programming** and **high performance computing**.  
In trading, C++ is mainly chosen for its performance and reliability.  
I want to make this simulator as fast as possible using **parallelization & distributed computing** techniques.

### How does it work?

##### 1- Data collection
  - Real data from the Yahoo finance API
  - Collected through **HTTP GET requests**

##### 2- Strategies Backtests
  - End user can use strategies like in the ./src/main.cpp program
  - 3 strategies have been coded in the strategies.cpp:
    *  **Dollar Cost Averaging** (DCA)
    *  Optimized DCA : investing on dips when the markets drops by 7% (arbitrary here but can be passed as an argument) below its simple moving average of the last x days (passed as argument)
    *  **LumpSum** strategy

##### 3- Save strategies
 - Each strategy is saved in the strat_outputs/ folder.
   * each row contains three information : Date, Portfolio Value & **Profit & Losses**
- Each strategy will display its **Total Return** (TR) (%) and its **Internal Rate of Return** (IRR) (%)


### To compile : 
*  in the src/ folder : g++ *.cpp -g -o main -lcurl
*  in the tst/ folder:  g++ -g *.cpp ../src/yahoo_timeseries.cpp ../src/portfolio_builder.cpp ../src/yahoo_utils.cpp -o main -lgtest -lcurl



### To come:
*  **Monte Carlo** simulation to get ideas of how likely a strategy will perform in the future
*  **Sharpe Ratio** optimizer to get the optimal distribution of the weights in a portfolio
*  Parallelization techniques to speed up computations
*  Potentially Graphical User Interface to enable end users defining their own strategy to backtest in a more user friendly way than coding.
    -  Several Timeseries metrics have already been implemented like **RSI**, **Simple Moving Average (SMA)**, **Exponential Moving Average (EMA)**, **Maximum Drawdowns**, ...
