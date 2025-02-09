#ifndef STRATEGY_CONFIG
#define STRATEGY_CONFIG

#include <string>
#include <vector>
#include <map>

struct GeneralParameters
{
    const std::vector<std::string> &tickers;
    const std::string strategy_name, start_date, end_date, ts_granularity;
    const double starting_amount;
    const double fees_per_trade;
    const double rebalancing_threshold;
    const std::map<std::string, double> &assets_desired_pct_allocations;
    const bool reinvestment_policy; // for dividends or bond yields
};

struct RecurrentInvestmentParameters
{
    const double recurrent_investment_amount;
    const int investment_nb_days_frequency; // invest every x days
    const std::string investment_week_day;  // invest on which week day ?
};

struct RiskParameters
{
    const double stop_loss_percentage;
    const double take_profit_percentage;
};

struct TechnicalIndicators
{
    const int rsi_period;
    const int sma_period;
    const double rsi_buy_threshold;
    const double rsi_sold_threshold;
    const double sma_buy_threshold;
    const double sma_sold_threshold;
};

struct StrategyConfig
{
    const GeneralParameters global_params;
    const RecurrentInvestmentParameters rinv_params;
    const RiskParameters risk_params;
    const TechnicalIndicators indicator_params;
};

#endif