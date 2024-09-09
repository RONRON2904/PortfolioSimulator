#include "../headers/yahoo_finance.hpp"
#include "../headers/strategy.hpp"

//g++ *.cpp -o main -lcurl

int main(){
    std::vector<std::string> tickers = {"IDUS.L", "CSSPX.MI"};
    YahooFinance* yf = new YahooFinance(tickers, "2010-06-01", "2024-08-31", "1d");
    std::vector<YahooTimeseries> tickers_ts_data = yf->get_tickers_ts_data();
    //yf->print_tickers_ts_data(tickers_ts_data);
    
    Strategy* strat1 = new DCA(tickers_ts_data, 81200, 2000.0, {{"IDUS.L", 1.0}}, 30, 0.01, "DCA_dist_2010_2024");
    strat1->run_strategy();
    strat1->save_end_portfolio();

    
    Strategy* strat2 = new DCA(tickers_ts_data, 81200, 2000.0, {{"CSSPX.MI", 1.0}}, 30, 0.01, "DCA_acc_2010_2024");
    strat2->run_strategy();
    strat2->save_end_portfolio();

    /*
    Strategy* strat2 = new LumpSum(tickers_ts_data, 248000, {{"UDVD.L", 1.0}}, 30, 0.01, "LumpSum_2015_2024");
    strat2->run_strategy();
    strat2->save_end_portfolio();
    
    Strategy* strat3 = new SmaOptimizedDCA(tickers_ts_data, 2000.0, {{"UDVD.L", 1.0}}, 30, 0.01, 5, "OptimizedDCA_2015_2024");
    strat3->run_strategy();
    strat3->save_end_portfolio();
    */
    delete yf;
    delete strat1;
    delete strat2;

    return EXIT_SUCCESS;
}