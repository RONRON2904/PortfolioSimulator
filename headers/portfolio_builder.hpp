#ifndef PORTFOLIO_BUILDER
#define PORTFOLIO_BUILDER

#include <vector>
#include "./yahoo_timeseries.hpp"
#include <map>
#include <set>

struct AssetHolding {
    YahooTimeseries ticker_yt;
    std::map<std::time_t, double> historical_cumulative_ticker_shares;
    std::map<std::time_t, double> historical_cumulative_ticker_expenses;
};

class PortfolioBuilder {

public:
    PortfolioBuilder();
    
    struct AssetHolding* get_asset(std::string ticker);
    const struct AssetHolding* get_asset(std::string ticker) const;
    const std::map<std::time_t, double>& get_portfolio_historical_cash_flow() const; 

    void buy(const YahooTimeseries& ticker_yt, double shares_amt, std::time_t date);
    void sell(const YahooTimeseries& ticker_yt, double shares_amt, std::time_t date);
    void set_portfolio_values_and_prices();
    void save_portfolio(std::string filename) const;
    
    double get_ticker_value(std::string ticker, std::time_t date) const;
    double get_ticker_shares(std::string ticker, std::time_t date) const;
    double get_ticker_expenses_value(std::string ticker, std::time_t date) const;
    double get_portfolio_value(std::time_t date) const;
    double get_portfolio_total_shares(std::time_t date) const;

    std::vector<std::time_t> get_unique_portfolio_dates() const;
    std::map<std::string, double> get_portfolio_percentage_allocations(std::time_t date) const;
    std::map<std::time_t, double> get_portfolio_values() const;
    
    Timeseries get_ticker_values(std::string ticker) const;
    Timeseries get_ts_portfolio_values() const;
    Timeseries get_ts_portfolio_prices() const;
    Timeseries get_ticker_profits_and_losses(std::string ticker) const;
    Timeseries get_portfolio_profits_and_losses() const;

    ~PortfolioBuilder();


private:
    std::vector<struct AssetHolding> assets;
    std::map<std::time_t, double> historical_cash_flow;
    std::map<std::time_t, double> portfolio_values;
    std::map<std::time_t, double> portfolio_total_shares;
    std::map<std::time_t, double> portfolio_prices;
};

#endif