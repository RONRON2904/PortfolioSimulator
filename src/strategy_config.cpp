#include "../headers/strategy_config.hpp"
#include "../headers/yahoo_utils.hpp"
#include <cassert>

StrategyConfig::StrategyConfig(const GeneralParameters &global_params, 
                               const RecurrentInvestmentParameters &rinv_params, 
                               const RiskParameters &risk_params, 
                               const TechnicalIndicators &indicator_params): global_params(global_params),
                                                                             rinv_params(rinv_params),
                                                                             risk_params(risk_params),
                                                                             indicator_params(indicator_params)
{
    assert(contains_all_tickers_yt_vectors(global_params.all_tickers_yt, rinv_params.rinv_tickers_yt) && "Tickers for reccuring investments are not included in the global tickers list");
    assert(contains_all_tickers_yt_vectors(global_params.all_tickers_yt, indicator_params.sma_tickers_yt) && "Tickers with sma conditions are not included in the global tickers list");
    assert(contains_all_tickers_yt_vectors(global_params.all_tickers_yt, indicator_params.rsi_tickers_yt) && "Tickers with rsi conditions are not included in the global tickers list");
    assert(contains_all_tickers_yt_vectors(global_params.all_tickers_yt, risk_params.risk_tickers_yt) && "Tickers with risk management conditions are not included in the global tickers list");
}