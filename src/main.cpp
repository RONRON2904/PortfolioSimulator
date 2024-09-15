#include "../headers/yahoo_finance.hpp"
#include "../headers/strategy.hpp"

//g++ *.cpp -o main -lcurl

int main(){
    std::vector<std::string> tickers = {"CSSPX.MI", "EGLN.L"}; //"IDUS.L"
    YahooFinance* yf = new YahooFinance(tickers, "2015-06-01", "2024-08-31", "1d");
    std::vector<YahooTimeseries> tickers_ts_data = yf->get_tickers_ts_data();
    yf->print_tickers_ts_data(tickers_ts_data);
    
    
    Strategy* strat = new DCA(tickers_ts_data, 81200, 2000.0, {{"CSSPX.MI", 0.75}, {"EGLN.L", 0.25}}, 30, 0.01, "DCA_SPGold_acc_2015_2024");
    strat->run_strategy();
    strat->save_end_portfolio();
    //strat->run_montecarlo_simulations(1000);

    
    delete yf;
    delete strat;

    return EXIT_SUCCESS;
}