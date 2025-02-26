#ifndef STRATEGY_CONFIG
#define STRATEGY_CONFIG

#include <string>
#include <vector>
#include <map>
#include "./yahoo_timeseries.hpp"

struct GeneralParameters
{
    const std::vector<YahooTimeseries> &all_tickers_yt;
    std::string strategy_name;
    double starting_amount;
    double fees_per_trade;
    double flat_tax;
    bool reinvestment_policy; // for dividends or bond yields
};

struct RecurrentInvestmentParameters
{
    const std::vector<YahooTimeseries> &rinv_tickers_yt;
    double recurrent_investment_amount = 0.0;
    size_t investment_nb_months_frequency = 0; // invest every x months
    size_t investment_week_day = 0;  // invest on which week day ?
    size_t investment_montly_weeknum = 0; // invest on which week of the month 1...4 
    double rebalancing_threshold = 0.0;
    size_t rebalancing_freq = 0;
    double starting_amount = 0.0;
    const std::map<std::string, double> &assets_desired_pct_allocations;

    RecurrentInvestmentParameters();
    RecurrentInvestmentParameters(const std::vector<YahooTimeseries> &rinv_tickers_yt,
                                  double recurrent_investment_amount,
                                  size_t investment_nb_months_frequency,
                                  size_t investment_montly_weeknum,
                                  size_t investment_week_day,
                                  double rebalancing_threshold,
                                  size_t rebalancing_freq,
                                  double starting_amount,
                                  const std::map<std::string, double> &assets_desired_pct_allocations);
};

struct RiskParameters
{
    const std::vector<YahooTimeseries> &risk_tickers_yt;
    size_t pct_changes_window = 0;
    double stop_loss_percentage = 0.0;
    double take_profit_percentage = 0.0;

    RiskParameters();
    RiskParameters(const std::vector<YahooTimeseries> &risk_tickers_yt,
                   size_t pct_changes_window,
                   double stop_loss_percentage,
                   double take_profit_percentage);
};

struct TechnicalIndicators
{
    const std::vector<YahooTimeseries> &rsi_tickers_yt;
    const std::vector<YahooTimeseries> &sma_tickers_yt;
    //const std::vector<YahooTimeseries> &rsi_sma_tickers_yt; // for applying both rsi & sma conditions before buying / selling an asset
    size_t rsi_period = 0;
    size_t long_sma_period = 0;
    size_t short_sma_period = 0;
    double rsi_buy_threshold = 0.0;
    double rsi_sell_threshold = 0.0;

    TechnicalIndicators();
    TechnicalIndicators(const std::vector<YahooTimeseries> &rsi_tickers_yt,
                        const std::vector<YahooTimeseries> &sma_tickers_yt,
                        //const std::vector<YahooTimeseries> &rsi_sma_tickers_yt; // for applying both rsi & sma conditions before buying / selling an asset
                        size_t rsi_period,
                        size_t long_sma_period,
                        size_t short_sma_period,
                        double rsi_buy_threshold,
                        double rsi_sell_threshold);
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
    std::map<std::string, std::map<time_t, double>> tech_ind_short_sma_values;
    std::map<std::string, std::map<time_t, double>> tech_ind_long_sma_values;
    std::map<std::string, std::map<time_t, double>> tech_ind_rsi_values;
    std::map<std::string, double> rinv_assets_starting_amounts;
};

#endif