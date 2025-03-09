#include "../headers/yahoo_finance.hpp"
#include "../headers/strategy.hpp"
#include "../headers/input_handler.hpp"
#include <iostream>

// g++ *.cpp -o main -lcurl
// g++ -fopenmp *.cpp -o main -lcurl -lmpi for parallelized version

int main(int argc, char* argv[])
{
    UserInputHandler *inputs = new UserInputHandler(argc, argv);
    GeneralParameters global_params = inputs->get_general_parameters();
    RecurrentInvestmentParameters rinv_params = inputs->get_rinv_parameters();
    RiskParameters risk_params = inputs->get_risk_parameters();
    TechnicalIndicators techind_params = inputs->get_technical_indicators();

    StrategyConfig config(global_params, rinv_params, risk_params, techind_params);
    CustomStrategy *strat = new CustomStrategy(config);

    strat->run_strategy();
    strat->save_end_portfolio(config.global_params.strategy_name);

    delete inputs;
    delete strat;
    return EXIT_SUCCESS;
}

/* PREVIOUS MAIN
int main()
{
    std::vector<std::string> tickers = {"AAPL", "EGLN.L", "CSSPX.MI"};
    YahooFinance *yf = new YahooFinance(tickers, "2010-06-01", "2025-03-04", "1d");
    std::vector<YahooTimeseries> tickers_ts_data = yf->get_tickers_ts_data();
    static std::vector<YahooTimeseries> EMPTY_YTIMESERIES;

    GeneralParameters global_params = {tickers_ts_data, "CustomRSISMA_2015_2025", 120000.0, 2500.0, 1.0, 0.3, true};
    std::map<std::string, double> assets_desired_pct_allocations = {{"CSSPX.MI", 0.80}, {"EGLN.L", 0.20}};
    //RecurrentInvestmentParameters rinv_params(tickers_ts_data, 120000.0, 2500.0, 1, 0, 5, 0.01, 90, assets_desired_pct_allocations);
    RecurrentInvestmentParameters rinv_params;
    RiskParameters risk_params;
    TechnicalIndicators indicator_params(EMPTY_YTIMESERIES, EMPTY_YTIMESERIES, tickers_ts_data, 14, 150, 20, 80, 0.0, 0.0, 120000.0);
    StrategyConfig config(global_params, rinv_params, risk_params, indicator_params);
    CustomStrategy *strat = new CustomStrategy(config);

    strat->run_strategy();
    strat->save_end_portfolio(config.global_params.strategy_name);
    //strat->run_montecarlo_simulations(1000);

    delete yf;
    delete strat;

    return EXIT_SUCCESS;
}*/