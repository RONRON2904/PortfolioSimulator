#include "../headers/strategy_config.hpp"
#include "../headers/yahoo_utils.hpp"
#include <cassert>
#include <algorithm>
#include <random>

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

    if (risk_params.risk_tickers_yt.size() > 0)
    {
        for (auto &ticker_yt: risk_params.risk_tickers_yt)
        {
            this->pct_changes_values[ticker_yt.get_ticker()] = ticker_yt.get_closes().get_ts_pct_changes();
        }
    }
}

RecurrentInvestmentParameters::RecurrentInvestmentParameters(const std::vector<YahooTimeseries> &rinv_tickers_yt,
                                                             double recurrent_investment_amount,
                                                             int investment_nb_months_frequency,
                                                             int investment_montly_weeknum,
                                                             int investment_week_day,
                                                             double rebalancing_threshold,
                                                             int rebalancing_freq,
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
    assert(investment_montly_weeknum > 0 && "investment week number of the month must be > 0 !\n");
    assert(investment_montly_weeknum < 5 && "investment week number of the month must be < 5 !\n");
    assert(investment_week_day > 0 && "investment week daymust be > 0 !\n");
    assert(investment_week_day < 6 && "investment week daymust be < 6 !\n");
    assert(rebalancing_threshold > 0 && "Rebalancing threshold must be > 0 !\n");
    assert(rebalancing_freq >= 0 && "Rebalancing threshold must be >= 0 !\n");
    assert(starting_amount >= 0 && "Starting amount must be >= 0 !\n");
}
