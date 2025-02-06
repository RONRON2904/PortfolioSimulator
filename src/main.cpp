#include "../headers/yahoo_finance.hpp"
#include "../headers/strategy.hpp"

//g++ *.cpp -o main -lcurl
//g++ -fopenmp *.cpp -o main -lcurl -lmpi for parallelized version

int main(){
    std::vector<std::string> tickers = {"CSSPX.MI", "EGLN.L"}; 
    YahooFinance* yf = new YahooFinance(tickers, "2015-06-01", "2025-01-25", "1d");
    std::vector<YahooTimeseries> tickers_ts_data = yf->get_tickers_ts_data();
    yf->print_tickers_ts_data(tickers_ts_data);
    
    
    Strategy* strat = new DCA(tickers_ts_data, 120200, 0.0, {{"CSSPX.MI", 0.80}, {"EGLN.L", 0.20}}, 120, 0.01, "LumpSum_SPGold_acc_2015_2025");
    strat->run_strategy();
    strat->save_end_portfolio();
    strat->run_montecarlo_simulations(1000);

    
    delete yf;
    delete strat;

    return EXIT_SUCCESS;
}