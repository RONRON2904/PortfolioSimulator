#include "../headers/yahoo_finance.hpp"
#include "../headers/strategy.hpp"

// g++ *.cpp -o main -lcurl
// g++ -fopenmp *.cpp -o main -lcurl -lmpi for parallelized version

int main()
{
    std::vector<std::string> tickers = {"CSSPX.MI", "EGLN.L"};
    YahooFinance *yf = new YahooFinance(tickers, "2015-06-01", "2025-02-28", "1d");
    std::vector<YahooTimeseries> tickers_ts_data = yf->get_tickers_ts_data();
    //yf->print_tickers_ts_data(tickers_ts_data);

    GeneralParameters global_params = {tickers_ts_data, "CustomDCA_2015_2025_Wn0_Wd5", 120000.0, 0.0, 1.0, 0.3, true};
    std::map<std::string, double> assets_desired_pct_allocations = {{"CSSPX.MI", 0.80}, {"EGLN.L", 0.20}};
    RecurrentInvestmentParameters rinv_params(tickers_ts_data, 120000.0, 2500.0, 1, 0, 5, 0.01, 90, assets_desired_pct_allocations);
    RiskParameters risk_params;
    TechnicalIndicators indicator_params;
    StrategyConfig config(global_params, rinv_params, risk_params, indicator_params);
    CustomStrategy *strat = new CustomStrategy(config);
    //Strategy *strat = new DCA(tickers_ts_data, 120200, 0.0, {{"CSSPX.MI", 0.80}, {"EGLN.L", 0.20}}, 120, 0.01, "LumpSum_SPGold_acc_2015_2025");
    strat->run_strategy();
    strat->save_end_portfolio(config.global_params.strategy_name);
    //strat->run_montecarlo_simulations(1000);

    delete yf;
    delete strat;

    return EXIT_SUCCESS;
}