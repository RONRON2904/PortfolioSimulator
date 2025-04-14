#include "../headers/strategy.hpp"
#include "../headers/yahoo_utils.hpp"
#include <cassert>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <limits>
#include <random>
#include <omp.h>

CustomStrategy::CustomStrategy(StrategyConfig &config) : config(config)
{
    PortfolioBuilder *ptf = new PortfolioBuilder();
    this->ptf = ptf;
}

void CustomStrategy::make_transactions(std::time_t date)
{
    this->handle_recurrent_investment_parameters(date);
    this->apply_technical_indicators(date);
    this->handle_risk_parameters(date);
    if (this->config.rinv_params.rebalancing_freq > 0 && this->config.rinv_params.last_rebalancing_nb_days == this->config.rinv_params.rebalancing_freq)
    {
        this->rebalance_portfolio(date);
        this->config.rinv_params.last_rebalancing_nb_days = 0;
    }
    else
        this->config.rinv_params.last_rebalancing_nb_days++;
}

std::map<time_t, std::vector<double>> CustomStrategy::run_strategy()
{
    std::vector<std::time_t> dates = get_unique_dates(this->config.global_params.all_tickers_yt);
    this->ptf->deposit(this->config.global_params.starting_amount, dates.front());
    for (const auto &date : dates)
    {
        if (this->config.global_params.start_date > date)
            continue;
        if (this->config.global_params.end_date < date)
            break;
        if (std::count(this->config.global_params.monthly_deposit_dates.begin(), this->config.global_params.monthly_deposit_dates.end(), date) > 0)
        {
            ptf->deposit(this->config.global_params.monthly_deposit_amount, date);
        }
        this->make_transactions(date);
    }
    this->ptf->set_portfolio_values_and_prices();
    return this->ptf->get_portfolio_values_and_pls();
}

const std::map<std::time_t, double> CustomStrategy::get_strategy_values() const
{
    return this->ptf->get_portfolio_values();
}

double CustomStrategy::get_strategy_total_returns() const
{
    double last_pf_value = this->ptf->get_portfolio_values().rbegin()->second;
    double last_pf_expense = 0.0;
    for (const auto &ticker_yt : this->config.global_params.all_tickers_yt)
    {
        last_pf_expense += this->ptf->get_ticker_expenses_value(ticker_yt.get_ticker(), ticker_yt.get_dates().back());
    }
    return last_pf_value / last_pf_expense - 1;
}

double CustomStrategy::get_strategy_extended_internal_return_rate(double tolerance, int max_iterations) const
{
    std::vector<std::time_t> dates = get_unique_dates(this->config.global_params.all_tickers_yt);

    std::vector<double> ptf_cash_flow;
    std::vector<time_t> ptf_cash_flow_dates;
    for (const auto &pair : this->ptf->get_portfolio_historical_cash_flow())
        if (abs(pair.second) > 1e-3)
        {
            ptf_cash_flow_dates.push_back(pair.first);
            ptf_cash_flow.push_back(pair.second);
        }
    ptf_cash_flow.push_back(this->ptf->get_portfolio_value(dates.back()));
    ptf_cash_flow_dates.push_back(dates.back());

    double lower_bound = -1.0;
    double upper_bound = 1.0;
    double rate = 0.0;
    std::time_t first_date = ptf_cash_flow_dates[0];

    for (int i = 0; i < max_iterations; ++i)
    {
        double npv = 0.0;
        rate = (lower_bound + upper_bound) / 2.0;
        for (size_t i = 0; i < ptf_cash_flow.size(); ++i)
        {
            double days = std::difftime(ptf_cash_flow_dates[i], first_date) / (60 * 60 * 24);
            npv += ptf_cash_flow[i] / pow(1.0 + rate, days / 365.0);
        }

        if (fabs(npv) < tolerance)
        {
            return rate;
        }

        if (npv > 0)
            lower_bound = rate;
        else
            upper_bound = rate;
    }

    return std::round(100.0 * rate) / 100.0;
}

double CustomStrategy::get_strategy_total_investments() const{
    double total_investments = 0.0;
    for (const auto &ticker_yt : this->config.global_params.all_tickers_yt)
    {
        total_investments += this->ptf->get_ticker_expenses_value(ticker_yt.get_ticker(), ticker_yt.get_dates().back());
    }
    return std::round(100.0 * total_investments) / 100.0;
}

double CustomStrategy::get_strategy_max_drawdown() const{
    Timeseries portfolio_values = this->ptf->get_ts_portfolio_values();
    std::vector<double> max_drawdowns = portfolio_values.get_maximum_drawdowns(portfolio_values.get_ts_values().size());
    return std::round(100.0 * max_drawdowns[0]) / 100.0;
}

void CustomStrategy::save_end_portfolio(std::string filename)
{
    this->ptf->save_portfolio(filename);
    double tr = 100 * this->get_strategy_total_returns();
    double xirr = 100 * this->get_strategy_extended_internal_return_rate(1e-3, 1000);
    double ptf_end_value = this->ptf->get_portfolio_values().rbegin()->second;
    //std::cout << "Strategy " + this->config.global_params.strategy_name + " Total Returns: " << std::ceil(tr * 100.0) / 100.0 << "% - Internal Rate of Return: " << std::ceil(xirr * 100.0) / 100.0 << "%" << " Portfolio End Value: " << ptf_end_value << std::endl;
}

const YahooTimeseries CustomStrategy::montecarlo_simulation(const std::vector<std::time_t> &future_dates)
{
    Timeseries portfolio_prices = this->ptf->get_ts_portfolio_prices();
    std::vector<double> pct_changes = portfolio_prices.get_pct_changes();
    double ptf_mean_return = std::accumulate(pct_changes.begin(), pct_changes.end(), 0.0) / pct_changes.size();
    double ptf_volatility = get_standard_deviation(pct_changes);

    std::random_device rd;
    std::mt19937 generator(rd());
    std::normal_distribution<double> normal_dist(ptf_mean_return, ptf_volatility);

    std::vector<double> future_prices(future_dates.size());
    future_prices[0] = portfolio_prices.get_ts_values().rbegin()->second;
    for (size_t i = 1; i < future_dates.size(); ++i)
    {
        future_prices[i] = future_prices[i - 1] + (future_prices[i - 1] * normal_dist(generator));
    }
    return YahooTimeseries("MonteCarloSimulationTicker", future_dates, future_prices, future_prices, future_prices, future_prices, future_prices);
}

void CustomStrategy::run_montecarlo_simulations(size_t nb_simu)
{
    std::time_t currentTime = std::time(nullptr); // get current date
    std::tm *tm_start = std::localtime(&currentTime);
    std::tm tm_end = *tm_start;
    // Add 20 years to the current year
    tm_end.tm_year += 20;

    std::time_t start = std::mktime(tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 1 + 252 * 20;
    std::vector<std::time_t> future_dates = generate_random_dates(count, start, end);

#pragma omp parallel for num_threads(6)
    for (size_t i = 0; i < nb_simu; ++i)
    {
        const YahooTimeseries yt = this->montecarlo_simulation(future_dates);
        CustomStrategy *strat = new CustomStrategy(config);
        strat->run_strategy();
        strat->save_end_portfolio(this->config.global_params.strategy_name + "_ms_" +std::to_string(i));
        delete strat;
    }
}

CustomStrategy::~CustomStrategy()
{
    delete this->ptf;
}

void CustomStrategy::handle_recurrent_investment_parameters(std::time_t date)
{
    for (auto &ticker_yt : this->config.rinv_params.rinv_tickers_yt)
    {
        std::string ticker = ticker_yt.get_ticker();
        double ticker_value = ticker_yt.get_closes().get_ts_value(date);
        if (this->config.rinv_params.recurrent_investment_amount >= 0)
        {
            std::vector<std::time_t> ticker_invest_dates = this->config.rinv_params.tickers_rinvestment_dates[ticker]; //this->config.tickers_rinvestment_dates[ticker];
            double alloc_pct = this->config.rinv_params.assets_desired_pct_allocations.at(ticker);

            double shares_amt = 0.0;
            double amount = alloc_pct * this->config.rinv_params.recurrent_investment_amount;

            if (std::count(ticker_invest_dates.begin(), ticker_invest_dates.end(), date) > 0)
            {
                if (this->config.rinv_params.rinv_assets_starting_amounts[ticker] > 0) //if (this->config.rinv_assets_starting_amounts[ticker] > 0)
                {
                    amount += this->config.rinv_params.rinv_assets_starting_amounts[ticker];//this->config.rinv_assets_starting_amounts[ticker];
                    this->config.rinv_params.rinv_assets_starting_amounts[ticker] = 0; //this->config.rinv_assets_starting_amounts[ticker] = 0;
                }
                shares_amt = amount / ticker_value;
                if (shares_amt > 0  && this->ptf->get_cash_amount(date) > 0.01)
                    this->ptf->buy(ticker_yt, shares_amt, date);
            }
        }
        std::map<std::time_t, double> dividends = ticker_yt.get_dividends().get_ts_values();
        if (dividends.size() > 0 && dividends.find(date) != dividends.end())
        {   
            double shares_amt = (1 - this->config.global_params.flat_tax) * dividends[date] * this->ptf->get_ticker_shares(ticker, date) / ticker_value;
            if (this->config.global_params.reinvestment_policy == true){
                this->ptf->receive_dividend(shares_amt * ticker_value, date);
                this->ptf->buy(ticker_yt, shares_amt, date);
            }
        }

        if (this->config.rinv_params.withdraw_pct > 0)
        {
            std::vector<std::time_t> ticker_withdrawal_dates = this->config.rinv_params.tickers_sell_dates_for_withdrawing[ticker];
            if (std::count(ticker_withdrawal_dates.begin(), ticker_withdrawal_dates.end(), date) > 0)
            {
                double ticker_shares_to_sell =  this->ptf->get_ticker_shares(ticker, date) * this->config.rinv_params.withdraw_pct / (12 / this->config.rinv_params.withdraw_nb_months_frequency);
                this->ptf->sell(ticker_yt, ticker_shares_to_sell, date);
                this->ptf->withdraw(ticker_shares_to_sell * ticker_value, date);
            }
        }

        if (this->config.rinv_params.withdraw_amount > 0)
        {
            std::vector<std::time_t> ticker_withdrawal_dates = this->config.rinv_params.tickers_sell_dates_for_withdrawing[ticker];
            if (std::count(ticker_withdrawal_dates.begin(), ticker_withdrawal_dates.end(), date) > 0)
            {
                double ticker_shares_to_sell =  (this->config.rinv_params.withdraw_amount / ticker_value) / (12 / this->config.rinv_params.withdraw_nb_months_frequency);
                this->ptf->sell(ticker_yt, ticker_shares_to_sell, date);
                this->ptf->withdraw(ticker_shares_to_sell * ticker_value, date);
            }
        }
    }
}

void CustomStrategy::apply_technical_indicators(std::time_t date)
{
    double cash_amt = this->ptf->get_cash_amount(date);
    double amount_per_ticker = cash_amt / this->config.indicator_params.sma_tickers_yt.size(); //Available cash is equally distributed accross the assets
    for (auto &ticker_yt : this->config.indicator_params.sma_tickers_yt)
    {
        std::string ticker = ticker_yt.get_ticker();
        double ticker_price = ticker_yt.get_closes().get_ts_value(date);
        double sma = this->config.indicator_params.tech_ind_sma_values[ticker][date];
        if (sma < ticker_price)
        {
            double shares_amt = amount_per_ticker / ticker_yt.get_closes().get_ts_value(date);
            this->ptf->buy(ticker_yt, shares_amt, date);
        }
        else if (sma > ticker_price) //sell it all
        {
            double ticker_shares = ptf->get_ticker_shares(ticker, date);
            this->ptf->sell(ticker_yt, ticker_shares, date);
        }
    }

    amount_per_ticker = cash_amt / this->config.indicator_params.rsi_tickers_yt.size(); //Available cash is equally distributed accross the assets
    for (auto &ticker_yt : this->config.indicator_params.rsi_tickers_yt)
    {
        std::string ticker = ticker_yt.get_ticker();
        double rsis = this->config.indicator_params.tech_ind_rsi_values[ticker][date];
        if (rsis < this->config.indicator_params.rsi_buy_threshold)
        {
            double shares_amt = amount_per_ticker / ticker_yt.get_closes().get_ts_value(date);
            this->ptf->buy(ticker_yt, shares_amt, date);
        }
        else if (rsis > this->config.indicator_params.rsi_sell_threshold) //sell it all
        {
            double ticker_shares = ptf->get_ticker_shares(ticker, date);
            this->ptf->sell(ticker_yt, ticker_shares, date);
        }
    }
    amount_per_ticker = cash_amt / this->config.indicator_params.rsi_sma_tickers_yt.size();
    for (auto &ticker_yt : this->config.indicator_params.rsi_sma_tickers_yt)
    {
        std::string ticker = ticker_yt.get_ticker();
        double ticker_price = ticker_yt.get_closes().get_ts_value(date);
        double rsis = this->config.indicator_params.tech_ind_rsi_values[ticker][date];
        double sma = this->config.indicator_params.tech_ind_sma_values[ticker][date];
        if (rsis < this->config.indicator_params.rsi_buy_threshold && sma < ticker_price)
        {
            double shares_amt = amount_per_ticker / ticker_yt.get_closes().get_ts_value(date);
            this->ptf->buy(ticker_yt, shares_amt, date);
        }
        else if (rsis > this->config.indicator_params.rsi_sell_threshold && sma > ticker_price) //sell it all
        {
            double ticker_shares = ptf->get_ticker_shares(ticker, date);
            this->ptf->sell(ticker_yt, ticker_shares, date);
        }
    }
}

void CustomStrategy::handle_risk_parameters(std::time_t date)
{
    for (auto &ticker_yt: this->config.risk_params.risk_tickers_yt)
    {
        std::string ticker = ticker_yt.get_ticker();
        double ticker_expense = this->ptf->get_ticker_expenses_value(ticker, date);
        double ticker_value = this->ptf->get_ticker_value(ticker, date);
        if ((ticker_value > (1 + this->config.risk_params.take_profit_percentage) * ticker_expense) || 
            (ticker_value < (1 - this->config.risk_params.stop_loss_percentage) * ticker_expense))
        {
            double ticker_shares = ptf->get_ticker_shares(ticker, date);
            this->ptf->sell(ticker_yt, ticker_shares, date);
        }
    }
}

void CustomStrategy::rebalance_portfolio(std::time_t date)
{
    std::map<std::string, double> ptf_alloc = this->ptf->get_portfolio_percentage_allocations(date);
    for (auto &ticker_yt : this->config.rinv_params.rinv_tickers_yt)
    {
        std::string ticker = ticker_yt.get_ticker();
        double ticker_shares = ptf->get_ticker_shares(ticker, date);
        double target_alloc = this->config.rinv_params.assets_desired_pct_allocations.at(ticker);
        double ticker_alloc = ptf_alloc[ticker];
        if (ticker_alloc - target_alloc > this->config.rinv_params.rebalancing_threshold && ticker_alloc > 0)
        {
            ticker_shares -= ticker_shares * target_alloc / ticker_alloc;
            this->ptf->sell(ticker_yt, ticker_shares, date);
        }
    }
    for (auto &ticker_yt : this->config.rinv_params.rinv_tickers_yt)
    {
        std::string ticker = ticker_yt.get_ticker();
        double ticker_shares = ptf->get_ticker_shares(ticker, date);
        double target_alloc = this->config.rinv_params.assets_desired_pct_allocations.at(ticker);
        double ticker_alloc = ptf_alloc[ticker];
        if (target_alloc - ticker_alloc > this->config.rinv_params.rebalancing_threshold && ticker_alloc > 0)
        {
            ticker_shares = (ticker_shares * target_alloc / ticker_alloc) - ticker_shares;
            this->ptf->buy(ticker_yt, ticker_shares, date);
        }
    }
}

Strategy::Strategy(const std::vector<YahooTimeseries> &tickers_yt, std::string strategy_name) : tickers_yt(tickers_yt),
                                                                                                strategy_name(strategy_name)
{
    PortfolioBuilder *ptf = new PortfolioBuilder();
    this->ptf = ptf;
}

void Strategy::run_strategy()
{
    std::vector<std::time_t> dates = get_unique_dates(this->tickers_yt);
    for (const auto &date : dates)
        this->make_transactions(date);
    this->ptf->set_portfolio_values_and_prices();
}

const std::map<std::time_t, double> Strategy::get_strategy_values() const
{
    return this->ptf->get_portfolio_values();
}

double Strategy::get_strategy_total_returns() const
{
    double last_pf_value = this->ptf->get_portfolio_values().rbegin()->second;
    double last_pf_expense = 0.0;
    for (const auto &ticker_yt : this->tickers_yt)
    {
        last_pf_expense += this->ptf->get_ticker_expenses_value(ticker_yt.get_ticker(), ticker_yt.get_dates().back());
    }
    return last_pf_value / last_pf_expense - 1;
}

double Strategy::get_strategy_extended_internal_return_rate(double tolerance, int max_iterations) const
{
    std::vector<std::time_t> dates = get_unique_dates(this->tickers_yt);

    std::vector<double> ptf_cash_flow;
    std::vector<time_t> ptf_cash_flow_dates;
    for (const auto &pair : this->ptf->get_portfolio_historical_cash_flow())
        if (abs(pair.second) > 1e-3)
        {
            ptf_cash_flow_dates.push_back(pair.first);
            ptf_cash_flow.push_back(pair.second);
        }
    ptf_cash_flow.push_back(this->ptf->get_portfolio_value(dates.back()));
    ptf_cash_flow_dates.push_back(dates.back());

    double lower_bound = -1.0;
    double upper_bound = 1.0;
    double rate = 0.0;
    std::time_t first_date = ptf_cash_flow_dates[0];

    for (int i = 0; i < max_iterations; ++i)
    {
        double npv = 0.0;
        rate = (lower_bound + upper_bound) / 2.0;
        for (size_t i = 0; i < ptf_cash_flow.size(); ++i)
        {
            double days = std::difftime(ptf_cash_flow_dates[i], first_date) / (60 * 60 * 24);
            npv += ptf_cash_flow[i] / pow(1.0 + rate, days / 365.0);
        }

        if (fabs(npv) < tolerance)
        {
            return rate;
        }

        if (npv > 0)
            lower_bound = rate;
        else
            upper_bound = rate;
    }

    return rate;
}

void Strategy::save_end_portfolio()
{
    this->ptf->save_portfolio(this->strategy_name);
    double tr = 100 * this->get_strategy_total_returns();
    double xirr = 100 * this->get_strategy_extended_internal_return_rate(1e-3, 1000);
    double ptf_end_value = this->ptf->get_portfolio_values().rbegin()->second;
    std::cout << "Strategy " + this->strategy_name + " Total Returns: " << std::ceil(tr * 100.0) / 100.0 << "% - Internal Rate of Return: " << std::ceil(xirr * 100.0) / 100.0 << "%" << " Portfolio End Value: " << ptf_end_value << std::endl;
}

const YahooTimeseries Strategy::montecarlo_simulation(const std::vector<std::time_t> &future_dates)
{
    Timeseries portfolio_prices = this->ptf->get_ts_portfolio_prices();
    std::vector<double> pct_changes = portfolio_prices.get_pct_changes();
    double ptf_mean_return = std::accumulate(pct_changes.begin(), pct_changes.end(), 0.0) / pct_changes.size();
    double ptf_volatility = get_standard_deviation(pct_changes);

    std::random_device rd;
    std::mt19937 generator(rd());
    std::normal_distribution<double> normal_dist(ptf_mean_return, ptf_volatility);

    std::vector<double> future_prices(future_dates.size());
    future_prices[0] = portfolio_prices.get_ts_values().rbegin()->second;
    for (size_t i = 1; i < future_dates.size(); ++i)
    {
        future_prices[i] = future_prices[i - 1] + (future_prices[i - 1] * normal_dist(generator));
    }
    return YahooTimeseries("MonteCarloSimulationTicker", future_dates, future_prices, future_prices, future_prices, future_prices, future_prices);
}

Strategy::~Strategy()
{
    delete this->ptf;
}

DCA::DCA(const std::vector<YahooTimeseries> &tickers_yt,
         double starting_amount,
         double recurrent_investment_amount,
         const std::map<std::string, double> &assets_desired_pct_allocations,
         int rebalancing_freq,
         double rebalancing_threshold,
         std::string strategy_name) : Strategy(tickers_yt, strategy_name),
                                      starting_amount(starting_amount),
                                      recurrent_investment_amount(recurrent_investment_amount),
                                      assets_desired_pct_allocations(assets_desired_pct_allocations),
                                      rebalancing_freq(rebalancing_freq),
                                      rebalancing_threshold(rebalancing_threshold),
                                      last_rebalancing_nb_days(0)
{
    std::vector<std::string> tickers;
    for (auto &ticker_yt : tickers_yt)
    {
        std::string ticker = ticker_yt.get_ticker();
        tickers.push_back(ticker);
        this->tickers_first_month_dates[ticker] = extract_first_dates_of_each_month(ticker_yt.get_dates());
        this->tickers_last_month_dates[ticker] = extract_last_dates_of_each_month(ticker_yt.get_dates());
    }
    double sum = 0.0;
    for (const auto &pair : assets_desired_pct_allocations)
    {
        assert(std::find(tickers.begin(), tickers.end(), pair.first) != tickers.end() && "pct allocation ticker name not in the passed YahooTimeries tickers list\n");
        assert(pair.second > 0 && "Each percentage allocation must be > 0!\n");
        sum += pair.second;
        this->assets_starting_amounts[pair.first] = assets_desired_pct_allocations.at(pair.first) * starting_amount;
    }

    assert(std::fabs(sum - 1.0) < 1e-9 && "The sum of percentages is not equal to 1!\n");
}

void DCA::rebalance_portfolio(std::time_t date)
{
    std::map<std::string, double> ptf_alloc = this->ptf->get_portfolio_percentage_allocations(date);
    for (auto &ticker_yt : this->tickers_yt)
    {
        std::string ticker = ticker_yt.get_ticker();
        double ticker_shares = ptf->get_ticker_shares(ticker, date);
        double target_alloc = this->assets_desired_pct_allocations[ticker];
        double ticker_alloc = ptf_alloc[ticker];
        if (ticker_alloc - target_alloc > this->rebalancing_threshold)
        {
            ticker_shares -= ticker_shares * target_alloc / ticker_alloc;
            this->ptf->sell(ticker_yt, ticker_shares, date);
        }
        if (target_alloc - ticker_alloc > this->rebalancing_threshold && ticker_alloc > 0)
        {
            ticker_shares = (ticker_shares * target_alloc / ticker_alloc) - ticker_shares;
            this->ptf->buy(ticker_yt, ticker_shares, date);
        }
    }
}

void DCA::make_transaction(const YahooTimeseries &ticker_yt, std::time_t date)
{
    std::string ticker = ticker_yt.get_ticker();
    std::vector<std::time_t> first_month_dates = this->tickers_first_month_dates[ticker];
    double alloc_pct = this->assets_desired_pct_allocations[ticker];

    double ticker_value = ticker_yt.get_closes().get_ts_value(date);
    double shares_amt = 0.0;
    double amount = alloc_pct * this->recurrent_investment_amount;

    if (std::count(first_month_dates.begin(), first_month_dates.end(), date) > 0)
    {
        if (this->assets_starting_amounts[ticker] > 0)
        {
            amount += this->assets_starting_amounts[ticker];
            this->assets_starting_amounts[ticker] = 0;
        }
        shares_amt = amount / ticker_value;
        if (shares_amt > 0)
            this->ptf->buy(ticker_yt, shares_amt, date);
    }

    std::map<std::time_t, double> dividends = ticker_yt.get_dividends().get_ts_values();

    if (dividends.size() > 0 && dividends.find(date) != dividends.end())
    {
        shares_amt = 0.7 * dividends[date] * this->ptf->get_ticker_shares(ticker, date) / ticker_value;
        this->ptf->buy(ticker_yt, shares_amt, date);
    }
}

void DCA::make_transactions(std::time_t date)
{
    for (const auto &ticker_yt : this->tickers_yt)
    {
        make_transaction(ticker_yt, date);
    }
    if (this->last_rebalancing_nb_days == this->rebalancing_freq)
    {
        this->rebalance_portfolio(date);
        this->last_rebalancing_nb_days = 0;
    }
    else
        this->last_rebalancing_nb_days++;
}

void DCA::run_montecarlo_simulations(size_t nb_simu)
{
    std::time_t currentTime = std::time(nullptr); // get current date
    std::tm *tm_start = std::localtime(&currentTime);
    std::tm tm_end = *tm_start;
    // Add 20 years to the current year
    tm_end.tm_year += 20;

    std::time_t start = std::mktime(tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 1 + 252 * 20;
    std::vector<std::time_t> future_dates = generate_random_dates(count, start, end);
#pragma omp parallel for num_threads(6)
    for (size_t i = 0; i < nb_simu; ++i)
    {
        const YahooTimeseries yt = this->montecarlo_simulation(future_dates);
        Strategy *strat = new DCA({yt},
                                  this->starting_amount,
                                  this->recurrent_investment_amount,
                                  {{"MonteCarloSimulationTicker", 1.0}},
                                  this->rebalancing_threshold,
                                  this->rebalancing_freq,
                                  this->strategy_name + "_MonteCarloSimu_n" + std::to_string(i + 1));
        strat->run_strategy();
        strat->save_end_portfolio();
        delete strat;
    }
}

SmaOptimizedDCA::SmaOptimizedDCA(const std::vector<YahooTimeseries> &tickers_yt,
                                 double starting_amount,
                                 double recurrent_investment_amount,
                                 const std::map<std::string, double> &assets_desired_pct_allocations,
                                 int rebalancing_freq,
                                 double rebalancing_threshold,
                                 int sma_window_size,
                                 std::string strategy_name) : DCA(tickers_yt, starting_amount, recurrent_investment_amount, assets_desired_pct_allocations, rebalancing_freq, rebalancing_threshold, strategy_name)
{
    for (auto &ticker_yt : tickers_yt)
    {
        std::string ticker = ticker_yt.get_ticker();
        this->tickers_last_month_dates[ticker] = extract_last_dates_of_each_month(ticker_yt.get_dates());
        this->current_tickers_remaining_investment_amount[ticker] = 0.0;
        this->tickers_sma[ticker] = Timeseries(ticker_yt.get_closes().get_ts_simple_moving_averages(sma_window_size));
    }
}

void SmaOptimizedDCA::make_transaction(const YahooTimeseries &ticker_yt, std::time_t date, const Timeseries &simple_moving_avergages)
{
    std::string ticker = ticker_yt.get_ticker();
    double sma_value = simple_moving_avergages.get_ts_value(date);
    double ticker_value = ticker_yt.get_closes().get_ts_value(date);

    std::vector<std::time_t> first_month_dates = this->tickers_first_month_dates[ticker];
    std::vector<std::time_t> last_month_dates = this->tickers_last_month_dates[ticker];
    double ticker_alloc = this->assets_desired_pct_allocations.at(ticker);
    double amount = ticker_alloc * this->recurrent_investment_amount;

    if (std::count(first_month_dates.begin(), first_month_dates.end(), date) > 0)
    {
        if (this->assets_starting_amounts[ticker] > 0)
        {
            amount += this->assets_starting_amounts[ticker];
            this->assets_starting_amounts[ticker] = 0;
        }
        this->current_tickers_remaining_investment_amount[ticker] = amount;
    }
    double shares_amt;
    if (sma_value > 0.0 && (sma_value - ticker_value) / sma_value > 0.07)
    {
        shares_amt = this->current_tickers_remaining_investment_amount[ticker] / ticker_value;
        this->ptf->buy(ticker_yt, shares_amt, date);
        this->current_tickers_remaining_investment_amount[ticker] = 0;
    }
    if (std::count(last_month_dates.begin(), last_month_dates.end(), date) > 0 && this->current_tickers_remaining_investment_amount[ticker] > 0.0)
    {
        shares_amt = this->current_tickers_remaining_investment_amount[ticker] / ticker_yt.get_closes().get_ts_value(date);
        this->ptf->buy(ticker_yt, shares_amt, date);
    }

    std::map<std::time_t, double> dividends = ticker_yt.get_dividends().get_ts_values();
    if (dividends.size() > 0 && dividends.find(date) != dividends.end())
    {
        shares_amt = 0.7 * dividends[date] * this->ptf->get_ticker_shares(ticker, date) / ticker_value;
        this->ptf->buy(ticker_yt, shares_amt, date);
    }
}

void SmaOptimizedDCA::make_transactions(std::time_t date)
{
    for (const auto &ticker_yt : this->tickers_yt)
    {
        make_transaction(ticker_yt, date, this->tickers_sma[ticker_yt.get_ticker()]);
    }
    if (this->last_rebalancing_nb_days == this->rebalancing_freq)
    {
        this->rebalance_portfolio(date);
        this->last_rebalancing_nb_days = 0;
    }
    else
        this->last_rebalancing_nb_days++;
}

LumpSum::LumpSum(const std::vector<YahooTimeseries> &tickers_yt,
                 double initial_investment_amount,
                 const std::map<std::string, double> &assets_desired_pct_allocations,
                 int rebalancing_freq,
                 double rebalancing_threshold,
                 std::string strategy_name) : Strategy(tickers_yt, strategy_name),
                                              initial_investment_amount(initial_investment_amount),
                                              assets_desired_pct_allocations(assets_desired_pct_allocations),
                                              rebalancing_freq(rebalancing_freq),
                                              rebalancing_threshold(rebalancing_threshold)
{
    std::vector<std::string> tickers;
    for (auto &ticker_yt : tickers_yt)
    {
        std::string ticker = ticker_yt.get_ticker();
        tickers.push_back(ticker);
        this->tickers_first_date[ticker] = ticker_yt.get_dates()[0];
    }
    double sum = 0.0;
    for (const auto &pair : assets_desired_pct_allocations)
    {
        assert(std::find(tickers.begin(), tickers.end(), pair.first) != tickers.end() && "pct allocation ticker name not in the passed YahooTimeries tickers list\n");
        assert(pair.second > 0 && "Each percentage allocation must be > 0!\n");
        sum += pair.second;
    }

    assert(std::fabs(sum - 1.0) < 1e-9 && "The sum of percentages is not equal to 1!\n");
    this->last_rebalancing_nb_days = 0;
}

void LumpSum::make_transaction(const YahooTimeseries &ticker_yt, std::time_t date)
{
    std::string ticker = ticker_yt.get_ticker();
    double alloc_pct = this->assets_desired_pct_allocations[ticker];
    double ticker_value = ticker_yt.get_closes().get_ts_value(date);
    double shares_amt = alloc_pct * this->initial_investment_amount / ticker_value;
    this->ptf->buy(ticker_yt, shares_amt, date);
}

void LumpSum::rebalance_portfolio(std::time_t date)
{
    std::map<std::string, double> ptf_alloc = this->ptf->get_portfolio_percentage_allocations(date);
    for (auto &ticker_yt : this->tickers_yt)
    {
        std::string ticker = ticker_yt.get_ticker();
        double ticker_shares = ptf->get_ticker_shares(ticker, date);
        double target_alloc = this->assets_desired_pct_allocations[ticker];
        double ticker_alloc = ptf_alloc[ticker];
        if ((ticker_alloc - target_alloc) / target_alloc > this->rebalancing_threshold)
        {
            ticker_shares -= ticker_shares * target_alloc / ticker_alloc;
            this->ptf->sell(ticker_yt, ticker_shares, date);
        }
        if (ticker_alloc > 0 && (target_alloc - ticker_alloc) / target_alloc > this->rebalancing_threshold)
        {
            ticker_shares = (ticker_shares * target_alloc / ticker_alloc) - ticker_shares;
            this->ptf->buy(ticker_yt, ticker_shares, date);
        }
    }
}

void LumpSum::make_transactions(std::time_t date)
{
    for (const auto &ticker_yt : this->tickers_yt)
    {
        std::string ticker = ticker_yt.get_ticker();
        if (date == this->tickers_first_date[ticker])
            make_transaction(ticker_yt, date);

        std::map<std::time_t, double> dividends = ticker_yt.get_dividends().get_ts_values();
        if (dividends.size() > 0 && dividends.find(date) != dividends.end())
        {
            double shares_amt = 0.7 * dividends[date] * this->ptf->get_ticker_shares(ticker, date) / ticker_yt.get_closes().get_ts_value(date);
            this->ptf->buy(ticker_yt, shares_amt, date);
        }
    }
    if (this->last_rebalancing_nb_days == this->rebalancing_freq)
    {
        this->rebalance_portfolio(date);
        this->last_rebalancing_nb_days = 0;
    }
    else
        this->last_rebalancing_nb_days++;
}