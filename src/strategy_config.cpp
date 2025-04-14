#include "../headers/strategy_config.hpp"
#include "../headers/yahoo_utils.hpp"
#include <cassert>
#include <algorithm>
#include <random>
#include <iostream>

static std::vector<YahooTimeseries> EMPTY_YTIMESERIES;
static std::map<std::string, double> EMPTY_MAP;

StrategyConfig::StrategyConfig(const GeneralParameters &global_params, 
                               RecurrentInvestmentParameters &rinv_params, 
                               RiskParameters &risk_params, 
                               TechnicalIndicators &indicator_params): global_params(global_params),
                                                                        rinv_params(rinv_params),
                                                                        risk_params(risk_params),
                                                                        indicator_params(indicator_params)
{
    assert(contains_all_tickers_yt_vectors(global_params.all_tickers_yt, rinv_params.rinv_tickers_yt) && "Tickers for reccuring investments are not included in the global tickers list");
    assert(contains_all_tickers_yt_vectors(global_params.all_tickers_yt, indicator_params.sma_tickers_yt) && "Tickers with sma conditions are not included in the global tickers list");
    assert(contains_all_tickers_yt_vectors(global_params.all_tickers_yt, indicator_params.rsi_tickers_yt) && "Tickers with rsi conditions are not included in the global tickers list");
    assert(contains_all_tickers_yt_vectors(global_params.all_tickers_yt, risk_params.risk_tickers_yt) && "Tickers with risk management conditions are not included in the global tickers list");
    assert(rinv_params.starting_amount <= global_params.starting_amount && "Recurrent & Technical investment starting amount sum can't be greater that global starting amount");
}

RecurrentInvestmentParameters::RecurrentInvestmentParameters(): rinv_tickers_yt(EMPTY_YTIMESERIES), assets_desired_pct_allocations(EMPTY_MAP) {}

RecurrentInvestmentParameters::RecurrentInvestmentParameters(const std::vector<YahooTimeseries> &rinv_tickers_yt,
                                                             double starting_amount,
                                                             double recurrent_investment_amount,
                                                             size_t investment_nb_months_frequency,
                                                             size_t investment_montly_weeknum,
                                                             size_t investment_week_day,
                                                             double rebalancing_threshold,
                                                             size_t rebalancing_freq,
                                                             const std::map<std::string, double> &assets_desired_pct_allocations,
                                                             double withdraw_pct,
                                                             double withdraw_amount,
                                                             size_t withdraw_nb_months_frequency,
                                                             size_t withdraw_monthly_weeknum,
                                                             size_t withdraw_week_day): 
                                                                                                                                rinv_tickers_yt(rinv_tickers_yt),
                                                                                                                                starting_amount(starting_amount),
                                                                                                                                recurrent_investment_amount(recurrent_investment_amount),
                                                                                                                                investment_nb_months_frequency(investment_nb_months_frequency),
                                                                                                                                investment_montly_weeknum(investment_montly_weeknum),
                                                                                                                                investment_week_day(investment_week_day),
                                                                                                                                rebalancing_threshold(rebalancing_threshold),
                                                                                                                                rebalancing_freq(rebalancing_freq),
                                                                                                                                assets_desired_pct_allocations(assets_desired_pct_allocations),
                                                                                                                                withdraw_pct(withdraw_pct),
                                                                                                                                withdraw_amount(withdraw_amount),
                                                                                                                                withdraw_nb_months_frequency(withdraw_nb_months_frequency),
                                                                                                                                withdraw_monthly_weeknum(withdraw_monthly_weeknum),
                                                                                                                                withdraw_week_day(withdraw_week_day)
{   
    assert(recurrent_investment_amount >= 0 && "Recurrent investment amount must be >= 0 !\n");
    assert(investment_nb_months_frequency >= 0 && "investment month freq must be >= 0 !\n");
    assert(investment_montly_weeknum >= 0 && "investment week number of the month must be >= 0 !\n");
    assert(investment_montly_weeknum < 5 && "investment week number of the month must be < 5 !\n");
    assert(investment_week_day > 0 && "investment week daymust be > 0 !\n");
    assert(investment_week_day < 6 && "investment week daymust be < 6 !\n");
    assert(rebalancing_threshold >= 0 && "Rebalancing threshold must be >= 0 !\n");
    assert(rebalancing_freq >= 0 && "Rebalancing threshold must be >= 0 !\n");
    assert(starting_amount >= 0 && "Starting amount must be >= 0 !\n");
    assert(withdraw_pct >= 0 && "Withdraw % must be >= 0 !\n");
    assert(withdraw_amount >= 0 && "Withdraw amount must be >= 0 !\n");
    assert(withdraw_nb_months_frequency >= 0 && "withdrawal month freq must be >= 0 !\n");
    assert(withdraw_monthly_weeknum >= 0 && "withdrawal week number of the month must be >= 0 !\n");
    assert(withdraw_monthly_weeknum < 5 && "withdrawal week number of the month must be < 5 !\n");
    assert(withdraw_week_day > 0 && "withdrawal week day must be > 0 !\n");
    assert(withdraw_week_day < 6 && "withdrawal week day must be < 6 !\n");

    if (rinv_tickers_yt.size() > 0)
    {
        std::vector<std::string> tickers;
        if (investment_nb_months_frequency + withdraw_nb_months_frequency > 0)
        {
            for (auto &ticker_yt : rinv_tickers_yt)
            {
                std::string ticker = ticker_yt.get_ticker();
                tickers.push_back(ticker);
                std::vector<time_t> ticker_yt_dates = ticker_yt.get_dates();
                if (investment_nb_months_frequency > 0)
                    this->tickers_rinvestment_dates[ticker] = extract_strategy_config_recurrent_investment_dates(ticker_yt_dates, 
                                                                                                                investment_nb_months_frequency, 
                                                                                                                investment_montly_weeknum,
                                                                                                                investment_week_day);
                if(withdraw_nb_months_frequency > 0)
                    this->tickers_sell_dates_for_withdrawing[ticker] = extract_strategy_config_recurrent_investment_dates(ticker_yt_dates, 
                                                                                                                        withdraw_nb_months_frequency, 
                                                                                                                        withdraw_monthly_weeknum,
                                                                                                                        withdraw_week_day);
            }
        }
        
        double sum = 0.0;
        for (const auto &pair : assets_desired_pct_allocations)
        {
            assert(std::find(tickers.begin(), tickers.end(), pair.first) != tickers.end() && "pct allocation ticker name not in the passed YahooTimeries tickers list\n");
            assert(pair.second > 0 && "Each percentage allocation must be > 0!\n");
            sum += pair.second;
            this->rinv_assets_starting_amounts[pair.first] = assets_desired_pct_allocations.at(pair.first) * starting_amount;
        }
        assert(std::fabs(sum - 1.0) < 1e-4 && "The sum of percentages is not equal to 1!\n");
    }
    this->last_rebalancing_nb_days = 0;
}

GeneralParameters::GeneralParameters(const std::vector<YahooTimeseries> &all_tickers_yt,
                                    std::time_t start_date,
                                    std::time_t end_date,
                                    std::string strategy_name,
                                    double starting_amount,
                                    double monthly_deposit_amount,
                                    double fees_per_trade,
                                    double flat_tax,
                                    bool reinvestment_policy):all_tickers_yt(all_tickers_yt),
                                                              start_date(start_date),
                                                              end_date(end_date),
                                                              strategy_name(strategy_name),
                                                              starting_amount(starting_amount),
                                                              monthly_deposit_amount(monthly_deposit_amount),
                                                              fees_per_trade(fees_per_trade),
                                                              flat_tax(flat_tax),
                                                              reinvestment_policy(reinvestment_policy)
{
    assert(all_tickers_yt.size() > 0 && "A strategy must contain at least one asset. \n");
    assert(starting_amount >= 0 && "Starting amount must be >= 0 !\n");
    assert(monthly_deposit_amount >= 0 && "Monthly deposit amount must be >= 0 !\n");
    assert(fees_per_trade >= 0 && "Monthly deposit amount must be > 0 !\n");
    assert(flat_tax >= 0 && "Flat tax must be >= 0 !\n");
    assert(flat_tax < 1 && "Flat tax must be < 1 !\n");

    std::vector<std::time_t> dates = get_unique_dates(all_tickers_yt);
    assert(dates.size() > 0 && "There are no dates in the passed YahooTimeseries. \n");
    //assert(start_date >= dates[0] && "Start date must be >= the first date of the passed YahooTimeseries. \n");
    //assert(end_date <= dates[dates.size() - 1] && "End date must be <= the last date of the passed YahooTimeseries. \n");
    assert(start_date < end_date && "Start date must be < end date. \n");
    this->monthly_deposit_dates = extract_first_dates_of_each_month(dates);
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

TechnicalIndicators::TechnicalIndicators(): rsi_tickers_yt(EMPTY_YTIMESERIES), sma_tickers_yt(EMPTY_YTIMESERIES), rsi_sma_tickers_yt(EMPTY_YTIMESERIES){}

TechnicalIndicators::TechnicalIndicators(const std::vector<YahooTimeseries> &rsi_tickers_yt,
                                         const std::vector<YahooTimeseries> &sma_tickers_yt,
                                         const std::vector<YahooTimeseries> &rsi_sma_tickers_yt,
                                         size_t rsi_period,
                                         size_t sma_period,
                                         double rsi_buy_threshold,
                                         double rsi_sell_threshold): rsi_tickers_yt(rsi_tickers_yt), 
                                                                     sma_tickers_yt(sma_tickers_yt),
                                                                     rsi_sma_tickers_yt(rsi_sma_tickers_yt),
                                                                     rsi_period(rsi_period),
                                                                     sma_period(sma_period),
                                                                     rsi_buy_threshold(rsi_buy_threshold),
                                                                     rsi_sell_threshold(rsi_sell_threshold)
{
    assert(rsi_tickers_yt.size() + sma_tickers_yt.size() + rsi_sma_tickers_yt.size() > 0 && "Configuring technical indicators needs to have assets \n");
    assert(rsi_buy_threshold >= 0  && "RSI buy for the rsi must be >= 0\n");
    assert(rsi_buy_threshold <= 100  && "RSI buy for the rsi must be <= 100\n");
    assert(rsi_sell_threshold >= 0  && "RSI sell threhold must be >= 0\n");
    assert(rsi_sell_threshold <= 100  && "RSI sell threhold must be <= 100\n");


    if (sma_tickers_yt.size() > 0)
    {
        assert(sma_period > 0  && "Window for the rsi must be > 0\n");
        for (auto &ticker_yt: sma_tickers_yt)
        {
            std::string ticker = ticker_yt.get_ticker();
            this->tech_ind_sma_values[ticker] = ticker_yt.get_closes().get_ts_simple_moving_averages(sma_period);
        }
    }

    if (rsi_tickers_yt.size() > 0)
    {
        assert(rsi_period > 0  && "Window for the rsi must be > 0\n");
        for (auto &ticker_yt: rsi_tickers_yt)
        {
            this->tech_ind_rsi_values[ticker_yt.get_ticker()] = ticker_yt.get_closes().get_ts_rsis(rsi_period);
        }
    }

    if (rsi_sma_tickers_yt.size() > 0)
    {
        assert(sma_period > 0  && "Window for the rsi must be > 0\n");
        assert(rsi_period > 0  && "Window for the rsi must be > 0\n");
        for (auto &ticker_yt: rsi_sma_tickers_yt)
        {
            std::string ticker = ticker_yt.get_ticker();
            this->tech_ind_sma_values[ticker] = ticker_yt.get_closes().get_ts_simple_moving_averages(sma_period);
            this->tech_ind_rsi_values[ticker] = ticker_yt.get_closes().get_ts_rsis(rsi_period);
        }
    }

}