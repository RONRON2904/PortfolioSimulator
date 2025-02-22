#ifndef STRATEGY_CONFIG
#define STRATEGY_CONFIG

#include <string>
#include <vector>
#include <map>

struct GeneralParameters
{
    std::vector<YahooTimeseries> &all_tickers_yt;
    std::string strategy_name, start_date, end_date, ts_granularity;
    double starting_amount;
    double fees_per_trade;
    double flat_tax;
    bool reinvestment_policy; // for dividends or bond yields
};

struct RecurrentInvestmentParameters
{
    const std::vector<YahooTimeseries> &rinv_tickers_yt;
    double recurrent_investment_amount;
    int investment_nb_months_frequency; // invest every x months
    int investment_week_day;  // invest on which week day ?
    int investment_montly_weeknum; // invest on which week of the month 1...4 
    double rebalancing_threshold;
    int rebalancing_freq;
    double starting_amount;
    const std::map<std::string, double> &assets_desired_pct_allocations;

    RecurrentInvestmentParameters(const std::vector<YahooTimeseries> &rinv_tickers_yt,
                                  double recurrent_investment_amount,
                                  int investment_nb_months_frequency,
                                  int investment_montly_weeknum,
                                  int investment_week_day,
                                  double rebalancing_threshold,
                                  int rebalancing_freq,
                                  double starting_amount,
                                  const std::map<std::string, double> &assets_desired_pct_allocations);
};

struct RiskParameters
{
    const std::vector<YahooTimeseries> &risk_tickers_yt;
    double stop_loss_percentage;
    double take_profit_percentage;
};

struct TechnicalIndicators
{
    const std::vector<YahooTimeseries> &rsi_tickers_yt;
    const std::vector<YahooTimeseries> &sma_tickers_yt;
    //const std::vector<YahooTimeseries> &rsi_sma_tickers_yt; // for applying both rsi & sma conditions before buying / selling an asset
    int rsi_period;
    int long_sma_period;
    int short_sma_period;
    double rsi_buy_threshold;
    double rsi_sell_threshold;
};

struct StrategyConfig
{
    const GeneralParameters &global_params;
    const RecurrentInvestmentParameters &rinv_params;
    const RiskParameters &risk_params;
    const TechnicalIndicators &indicator_params;
    StrategyConfig(const GeneralParameters &global_params, 
                   const RecurrentInvestmentParameters &rinv_params, 
                   const RiskParameters &risk_params, 
                   const TechnicalIndicators &indicator_params);
    
    std::map<std::string, std::vector<std::time_t>> tickers_rinvestment_dates;
    std::map<std::string, double> rinv_assets_starting_amounts;
    std::map<std::string, std::map<time_t, double>> tech_ind_short_sma_values;
    std::map<std::string, std::map<time_t, double>> tech_ind_long_sma_values;
    std::map<std::string, std::map<time_t, double>> tech_ind_rsi_values;
    std::map<std::string, std::map<time_t, double>> pct_changes_values;
};

#endif