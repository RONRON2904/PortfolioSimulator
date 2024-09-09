#include "../headers/strategy.hpp"
#include "../headers/yahoo_utils.hpp"
#include <cassert>
#include <iostream>
#include <algorithm>
#include <limits>

Strategy::Strategy(const std::vector<YahooTimeseries>& tickers_yt, std::string strategy_name) : tickers_yt(tickers_yt), 
                                                                                                strategy_name(strategy_name){
    PortfolioBuilder* ptf = new PortfolioBuilder();
    this->ptf = ptf;
}

void Strategy::run_strategy(){
    std::vector<std::time_t> dates = get_unique_dates(this->tickers_yt);
    for (const auto& date: dates)
        this->make_transactions(date);
    this->ptf->set_portfolio_values();
}

double Strategy::get_strategy_total_returns() const{
    double last_pf_value = this->ptf->get_portfolio_values().rbegin()->second;
    double last_pf_expense = 0.0;
    for (const auto& ticker_yt: this->tickers_yt){
        last_pf_expense += this->ptf->get_ticker_expenses_value(ticker_yt.get_ticker(), ticker_yt.get_dates().back());
    }
    return last_pf_value / last_pf_expense - 1;
}

double Strategy::get_strategy_extended_internal_return_rate(double tolerance, int max_iterations) const{
    std::vector<std::time_t> dates = get_unique_dates(this->tickers_yt);
    
    std::vector<double> ptf_cash_flow;
    std::vector<time_t> ptf_cash_flow_dates;
    for (const auto& pair: this->ptf->get_portfolio_historical_cash_flow())
        if (abs(pair.second) > 1e-3){
            ptf_cash_flow_dates.push_back(pair.first);
            ptf_cash_flow.push_back(pair.second);
        }
    ptf_cash_flow.push_back(this->ptf->get_portfolio_value(dates.back()));
    ptf_cash_flow_dates.push_back(dates.back());

    double lower_bound = -1.0;
    double upper_bound = 1.0;
    double rate = 0.0;
    std::time_t first_date = ptf_cash_flow_dates[0];

    for (int i = 0; i < max_iterations; ++i) {
        double npv = 0.0;
        rate = (lower_bound + upper_bound) / 2.0;
        for (size_t i = 0; i < ptf_cash_flow.size(); ++i) {
            double days = std::difftime(ptf_cash_flow_dates[i], first_date) / (60 * 60 * 24);
            npv += ptf_cash_flow[i] / pow(1.0 + rate, days / 365.0);
        }

        if (fabs(npv) < tolerance) {
            return rate;
        }

        if (npv > 0)
            lower_bound = rate;
        else
            upper_bound = rate;        
    }

    return rate;
}

void Strategy::save_end_portfolio(){
    this->ptf->save_portfolio(this->strategy_name);
    double tr = 100 * this->get_strategy_total_returns();
    double xirr = 100 * this->get_strategy_extended_internal_return_rate(1e-3, 1000);
    double ptf_end_value = this->ptf->get_portfolio_values().rbegin()->second;
    std::cout << "Strategy "+ this->strategy_name+" Total Returns: " << std::ceil(tr * 100.0) / 100.0 << "% - Internal Rate of Return: " << std::ceil(xirr * 100.0) / 100.0 << "%" << " Portfolio End Value: " << ptf_end_value << std::endl;
}

Strategy::~Strategy(){
    delete this->ptf;
}



DCA::DCA(const std::vector<YahooTimeseries>& tickers_yt,
                           double starting_amount,
                           double recurrent_investment_amount,
                           const std::map<std::string, double>& assets_desired_pct_allocations,
                           int rebalancing_freq,
                           double rebalancing_threshold,
                           std::string strategy_name):Strategy(tickers_yt, strategy_name),
                                                        starting_amount(starting_amount),
                                                        recurrent_investment_amount(recurrent_investment_amount),
                                                        assets_desired_pct_allocations(assets_desired_pct_allocations),
                                                        rebalancing_freq(rebalancing_freq),
                                                        rebalancing_threshold(rebalancing_threshold){
    std::vector<std::string> tickers;
    for (auto& ticker_yt: tickers_yt){
        std::string ticker = ticker_yt.get_ticker();
        tickers.push_back(ticker);
        this->tickers_first_month_dates[ticker] = extract_first_dates_of_each_month(ticker_yt.get_dates());
        this->tickers_last_month_dates[ticker] = extract_last_dates_of_each_month(ticker_yt.get_dates());
    }    
    double sum = 0.0;
    for (const auto& pair : assets_desired_pct_allocations) {
        assert(std::find(tickers.begin(), tickers.end(), pair.first) != tickers.end() && "pct allocation ticker name not in the passed YahooTimeries tickers list\n");
        assert(pair.second > 0 && "Each percentage allocation must be > 0!\n");
        sum += pair.second;
    }

    assert(std::fabs(sum - 1.0) < 1e-9 && "The sum of percentages is not equal to 1!\n");
    this->last_rebalancing_nb_days = 0;
}

void DCA::rebalance_portfolio(std::time_t date){
    std::map<std::string, double> ptf_alloc = this->ptf->get_portfolio_percentage_allocations(date);
    for (auto& ticker_yt: this->tickers_yt){
        std::string ticker = ticker_yt.get_ticker();
        double ticker_shares = ptf->get_ticker_shares(ticker, date);
        double target_alloc = this->assets_desired_pct_allocations[ticker];
        double ticker_alloc = ptf_alloc[ticker];
        if (ticker_alloc - target_alloc > this->rebalancing_threshold){
            ticker_shares -= ticker_shares * target_alloc / ticker_alloc;
            this->ptf->sell(ticker_yt, ticker_shares, date);
        }
        if (target_alloc - ticker_alloc > this->rebalancing_threshold && ticker_alloc > 0){
            ticker_shares = (ticker_shares * target_alloc / ticker_alloc) - ticker_shares;
            this->ptf->buy(ticker_yt, ticker_shares, date);
        }
    }
}

void DCA::make_transaction(const YahooTimeseries& ticker_yt, std::time_t date) {
    std::string ticker = ticker_yt.get_ticker();
    std::vector<std::time_t> first_month_dates = this->tickers_first_month_dates[ticker];
    double alloc_pct = this->assets_desired_pct_allocations[ticker];

    double ticker_value = ticker_yt.get_closes().get_ts_value(date);
    double shares_amt = 0.0;
    double amount = this->recurrent_investment_amount + this->starting_amount;
    if (std::count(first_month_dates.begin(), first_month_dates.end(), date) > 0){
        shares_amt = alloc_pct * amount / ticker_value;
        if (shares_amt > 0){
            this->ptf->buy(ticker_yt, shares_amt, date);
            this->starting_amount = 0;
        }
    }

    std::map<std::time_t, double> dividends = ticker_yt.get_dividends().get_ts_values();

    if (dividends.size() > 0 && dividends.find(date) != dividends.end()){
        shares_amt = 0.7 * dividends[date] * this->ptf->get_ticker_shares(ticker, date) / ticker_value;
        this->ptf->buy(ticker_yt, shares_amt, date);
    }
}

void DCA::make_transactions(std::time_t date){
    for (const auto& ticker_yt: this->tickers_yt){
        make_transaction(ticker_yt, date);
    }
    if (this->last_rebalancing_nb_days == this->rebalancing_freq){
        this->rebalance_portfolio(date);
        this->last_rebalancing_nb_days = 0;
    }
    else
        this->last_rebalancing_nb_days++;
}

SmaOptimizedDCA::SmaOptimizedDCA(const std::vector<YahooTimeseries>& tickers_yt, 
                 double starting_amount,
                 double recurrent_investment_amount,
                 const std::map<std::string, double>& assets_desired_pct_allocations, 
                 int rebalancing_freq,
                 double rebalancing_threshold,
                 int sma_window_size,
                 std::string strategy_name):DCA(tickers_yt, starting_amount, recurrent_investment_amount, assets_desired_pct_allocations, rebalancing_freq, rebalancing_threshold, strategy_name){
    for (auto& ticker_yt: tickers_yt){
        std::string ticker = ticker_yt.get_ticker();
        this->tickers_last_month_dates[ticker] = extract_last_dates_of_each_month(ticker_yt.get_dates());
        this->current_tickers_remaining_investment_amount[ticker] = 0.0;
        this->tickers_sma[ticker] = Timeseries(ticker_yt.get_closes().get_ts_simple_moving_averages(sma_window_size));
    }    
}

void SmaOptimizedDCA::make_transaction(const YahooTimeseries& ticker_yt, std::time_t date, const Timeseries& simple_moving_avergages){
    std::string ticker = ticker_yt.get_ticker();
    double sma_value  = simple_moving_avergages.get_ts_value(date);
    double ticker_value =  ticker_yt.get_closes().get_ts_value(date);
   
    std::vector<std::time_t> first_month_dates = this->tickers_first_month_dates[ticker];
    std::vector<std::time_t> last_month_dates = this->tickers_last_month_dates[ticker];
    double amount = this->recurrent_investment_amount + this->starting_amount;
    if (std::count(first_month_dates.begin(), first_month_dates.end(), date) > 0){
        this->current_tickers_remaining_investment_amount[ticker] = this->assets_desired_pct_allocations.at(ticker) * amount;

        this->starting_amount = 0;
    }
    double shares_amt;
    if (sma_value > 0.0 && (sma_value - ticker_value) / sma_value > 0.07){
        shares_amt = this->current_tickers_remaining_investment_amount[ticker] / ticker_value;
        this->ptf->buy(ticker_yt, shares_amt, date);
        this->current_tickers_remaining_investment_amount[ticker] = 0;
    }
    if (std::count(last_month_dates.begin(), last_month_dates.end(), date) > 0 && this->current_tickers_remaining_investment_amount[ticker] > 0.0){
        shares_amt = this->current_tickers_remaining_investment_amount[ticker] / ticker_yt.get_closes().get_ts_value(date);
        this->ptf->buy(ticker_yt, shares_amt, date);
    }

    std::map<std::time_t, double> dividends = ticker_yt.get_dividends().get_ts_values();
    if (dividends.size() > 0 && dividends.find(date) != dividends.end()){
        shares_amt = 0.7 * dividends[date] * this->ptf->get_ticker_shares(ticker, date) / ticker_value;
        this->ptf->buy(ticker_yt, shares_amt, date);
    }
}

void SmaOptimizedDCA::make_transactions(std::time_t date){
    for (const auto& ticker_yt: this->tickers_yt){
        make_transaction(ticker_yt, date, this->tickers_sma[ticker_yt.get_ticker()]);
    }
    if (this->last_rebalancing_nb_days == this->rebalancing_freq){
        this->rebalance_portfolio(date);
        this->last_rebalancing_nb_days = 0;
    }
    else
        this->last_rebalancing_nb_days++;
}


LumpSum::LumpSum(const std::vector<YahooTimeseries>& tickers_yt,
                                double initial_investment_amount,
                                const std::map<std::string, double>& assets_desired_pct_allocations,
                                int rebalancing_freq,
                                double rebalancing_threshold,
                                std::string strategy_name):Strategy(tickers_yt, strategy_name), 
                                                                initial_investment_amount(initial_investment_amount),
                                                                assets_desired_pct_allocations(assets_desired_pct_allocations),
                                                                rebalancing_freq(rebalancing_freq),
                                                                rebalancing_threshold(rebalancing_threshold){
    std::vector<std::string> tickers;
    for (auto& ticker_yt: tickers_yt){
        std::string ticker = ticker_yt.get_ticker();
        tickers.push_back(ticker);
        this->tickers_first_date[ticker] = ticker_yt.get_dates()[0];
    }    
    double sum = 0.0;
    for (const auto& pair : assets_desired_pct_allocations) {
        assert(std::find(tickers.begin(), tickers.end(), pair.first) != tickers.end() && "pct allocation ticker name not in the passed YahooTimeries tickers list\n");
        assert(pair.second > 0 && "Each percentage allocation must be > 0!\n");
        sum += pair.second;
    }

    assert(std::fabs(sum - 1.0) < 1e-9 && "The sum of percentages is not equal to 1!\n");
    this->last_rebalancing_nb_days = 0;
}

void LumpSum::make_transaction(const YahooTimeseries& ticker_yt, std::time_t date) {
    std::string ticker = ticker_yt.get_ticker();
    double alloc_pct = this->assets_desired_pct_allocations[ticker];
    double ticker_value = ticker_yt.get_closes().get_ts_value(date);
    double shares_amt = alloc_pct * this->initial_investment_amount / ticker_value;
    this->ptf->buy(ticker_yt, shares_amt, date);
}

void LumpSum::rebalance_portfolio(std::time_t date){
    std::map<std::string, double> ptf_alloc = this->ptf->get_portfolio_percentage_allocations(date);
    for (auto& ticker_yt: this->tickers_yt){
        std::string ticker = ticker_yt.get_ticker();
        double ticker_shares = ptf->get_ticker_shares(ticker, date);
        double target_alloc = this->assets_desired_pct_allocations[ticker];
        double ticker_alloc = ptf_alloc[ticker];
        if ((ticker_alloc - target_alloc) / target_alloc > this->rebalancing_threshold){
            ticker_shares -= ticker_shares * target_alloc / ticker_alloc;
            this->ptf->sell(ticker_yt, ticker_shares, date);
        }
        if (ticker_alloc > 0 && (target_alloc - ticker_alloc) / target_alloc > this->rebalancing_threshold ){
            ticker_shares = (ticker_shares * target_alloc / ticker_alloc) - ticker_shares;
            this->ptf->buy(ticker_yt, ticker_shares, date);
        }
    }
}

void LumpSum::make_transactions(std::time_t date){
    for (const auto& ticker_yt: this->tickers_yt){
        std::string ticker = ticker_yt.get_ticker();
        if (date == this->tickers_first_date[ticker])
            make_transaction(ticker_yt, date);
        
        std::map<std::time_t, double> dividends = ticker_yt.get_dividends().get_ts_values();
        if (dividends.size() > 0 && dividends.find(date) != dividends.end()){
            double shares_amt = 0.7 * dividends[date] * this->ptf->get_ticker_shares(ticker, date) / ticker_yt.get_closes().get_ts_value(date);
            this->ptf->buy(ticker_yt, shares_amt, date);
        }
    }
    if (this->last_rebalancing_nb_days == this->rebalancing_freq){
        this->rebalance_portfolio(date);
        this->last_rebalancing_nb_days = 0;
    }
    else
        this->last_rebalancing_nb_days++;
}