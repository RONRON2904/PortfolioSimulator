#include "../headers/strategy_config.hpp"
#include "../headers/yahoo_utils.hpp"
#include <cassert>
#include <algorithm>
#include <random>
#include <iostream>

static std::vector<YahooTimeseries> EMPTY_YTIMESERIES;
static std::map<std::string, double> EMPTY_MAP;

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

    if (rinv_params.rinv_tickers_yt.size() > 0)
    {
        std::vector<std::string> tickers;
        for (auto &ticker_yt : rinv_params.rinv_tickers_yt)
        {
            std::string ticker = ticker_yt.get_ticker();
            tickers.push_back(ticker);
            this->tickers_rinvestment_dates[ticker] = extract_strategy_config_recurrent_investment_dates(ticker_yt.get_dates(), 
                                                                                                        rinv_params.investment_nb_months_frequency, 
                                                                                                        rinv_params.investment_montly_weeknum,
                                                                                                        rinv_params.investment_week_day);
        }
        double sum = 0.0;
        for (const auto &pair : rinv_params.assets_desired_pct_allocations)
        {
            std::cout << "PAIR FIRST: " + pair.first << std::endl;
            assert(std::find(tickers.begin(), tickers.end(), pair.first) != tickers.end() && "pct allocation ticker name not in the passed YahooTimeries tickers list\n");
            assert(pair.second > 0 && "Each percentage allocation must be > 0!\n");
            sum += pair.second;
            this->rinv_assets_starting_amounts[pair.first] = rinv_params.assets_desired_pct_allocations.at(pair.first) * rinv_params.starting_amount;
        }
        assert(std::fabs(sum - 1.0) < 1e-9 && "The sum of percentages is not equal to 1!\n");
    }

    if (indicator_params.sma_tickers_yt.size() > 0)
    {
        for (auto &ticker_yt: indicator_params.sma_tickers_yt)
        {
            std::string ticker = ticker_yt.get_ticker();
            this->tech_ind_short_sma_values[ticker] = ticker_yt.get_closes().get_ts_simple_moving_averages(indicator_params.short_sma_period);
            this->tech_ind_long_sma_values[ticker] = ticker_yt.get_closes().get_ts_simple_moving_averages(indicator_params.long_sma_period);
        }
    }

    if (indicator_params.rsi_tickers_yt.size() > 0)
    {
        for (auto &ticker_yt: indicator_params.rsi_tickers_yt)
        {
            this->tech_ind_rsi_values[ticker_yt.get_ticker()] = ticker_yt.get_closes().get_ts_rsis(indicator_params.rsi_period);
        }
    }
}

RecurrentInvestmentParameters::RecurrentInvestmentParameters(): rinv_tickers_yt(EMPTY_YTIMESERIES), assets_desired_pct_allocations(EMPTY_MAP) {}

RecurrentInvestmentParameters::RecurrentInvestmentParameters(const std::vector<YahooTimeseries> &rinv_tickers_yt,
                                                             double recurrent_investment_amount,
                                                             size_t investment_nb_months_frequency,
                                                             size_t investment_montly_weeknum,
                                                             size_t investment_week_day,
                                                             double rebalancing_threshold,
                                                             size_t rebalancing_freq,
                                                             double starting_amount,
                                                             const std::map<std::string, double> &assets_desired_pct_allocations): 
                                                                                                                                rinv_tickers_yt(rinv_tickers_yt),
                                                                                                                                recurrent_investment_amount(recurrent_investment_amount),
                                                                                                                                investment_nb_months_frequency(investment_nb_months_frequency),
                                                                                                                                investment_montly_weeknum(investment_montly_weeknum),
                                                                                                                                investment_week_day(investment_week_day),
                                                                                                                                rebalancing_threshold(rebalancing_threshold),
                                                                                                                                rebalancing_freq(rebalancing_freq),
                                                                                                                                starting_amount(starting_amount),
                                                                                                                                assets_desired_pct_allocations(assets_desired_pct_allocations)
{   
    assert(recurrent_investment_amount > 0 && "Recurrent investment amount must be > 0 !\n");
    assert(investment_nb_months_frequency > 0 && "investment month freq must be > 0 !\n");
    assert(investment_montly_weeknum >= 0 && "investment week number of the month must be >= 0 !\n");
    assert(investment_montly_weeknum < 5 && "investment week number of the month must be < 5 !\n");
    assert(investment_week_day >= 0 && "investment week daymust be >= 0 !\n");
    assert(investment_week_day < 6 && "investment week daymust be < 6 !\n");
    assert(rebalancing_threshold > 0 && "Rebalancing threshold must be > 0 !\n");
    assert(rebalancing_freq >= 0 && "Rebalancing threshold must be >= 0 !\n");
    assert(starting_amount >= 0 && "Starting amount must be >= 0 !\n");
}

RiskParameters::RiskParameters(const std::vector<YahooTimeseries> &risk_tickers_yt,
                               size_t pct_changes_window,
                               double stop_loss_percentage,
                               double take_profit_percentage): risk_tickers_yt(risk_tickers_yt), 
                                                               pct_changes_window(pct_changes_window),
                                                               stop_loss_percentage(stop_loss_percentage),
                                                               take_profit_percentage(take_profit_percentage)
{
    assert(pct_changes_window > 0 && "Window for the pct of change must be > 0");
    assert(stop_loss_percentage > 0 && "Stop Loss pct must be > 0");
    assert(take_profit_percentage > 0 && "Take profit pct must be > 0");
}

RiskParameters::RiskParameters(): risk_tickers_yt(EMPTY_YTIMESERIES){} // Avoid dangling reference

TechnicalIndicators::TechnicalIndicators(): rsi_tickers_yt(EMPTY_YTIMESERIES), sma_tickers_yt(EMPTY_YTIMESERIES){}

TechnicalIndicators::TechnicalIndicators(const std::vector<YahooTimeseries> &rsi_tickers_yt,
                                         const std::vector<YahooTimeseries> &sma_tickers_yt,
                                         //const std::vector<YahooTimeseries> &rsi_sma_tickers_yt; // for applying both rsi & sma conditions before buying / selling an asset
                                         size_t rsi_period,
                                         size_t long_sma_period,
                                         size_t short_sma_period,
                                         double rsi_buy_threshold,
                                         double rsi_sell_threshold): rsi_tickers_yt(rsi_tickers_yt), 
                                                                     sma_tickers_yt(sma_tickers_yt),
                                                                     rsi_period(rsi_period),
                                                                     long_sma_period(long_sma_period),
                                                                     short_sma_period(short_sma_period),
                                                                     rsi_buy_threshold(rsi_buy_threshold),
                                                                     rsi_sell_threshold(rsi_sell_threshold)
{
    assert(rsi_period > 0  && "Window for the rsi must be > 0");
    assert(long_sma_period > 0  && "Window for the rsi must be > 0");
    assert(short_sma_period > 0  && "Window for the rsi must be > 0");
    assert(rsi_buy_threshold >= 0  && "RSI buy for the rsi must be >= 0");
    assert(rsi_buy_threshold <= 100  && "RSI buy for the rsi must be <= 100");
    assert(rsi_sell_threshold >= 0  && "RSI sell threhold must be >= 0");
    assert(rsi_sell_threshold <= 100  && "RSI sell threhold must be <= 100");

}