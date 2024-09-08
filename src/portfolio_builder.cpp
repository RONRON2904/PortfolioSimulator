#include "../headers/portfolio_builder.hpp"
#include "../headers/yahoo_utils.hpp"
#include <eigen3/Eigen/Dense>
#include <iostream>
#include <fstream>
#include <chrono>

struct AssetHolding* PortfolioBuilder::get_asset(std::string ticker) {
    for (auto& asset : this->assets)
        if (asset.ticker_yt.get_ticker() == ticker)
            return &asset;
    return nullptr;
}

const struct AssetHolding* PortfolioBuilder::get_asset(std::string ticker) const{
    for (auto& asset : this->assets)
        if (asset.ticker_yt.get_ticker() == ticker)
            return &asset;
    return nullptr;
}

const std::map<std::time_t, double>& PortfolioBuilder::get_portfolio_historical_cash_flow() const {
    return this->historical_cash_flow;
}

std::vector<std::time_t> PortfolioBuilder::get_unique_portfolio_dates() const{
    std::set<std::time_t> unique_dates;
    for (auto& asset : this->assets)
         for (const auto& ticker_date : asset.ticker_yt.get_dates())
            unique_dates.insert(ticker_date);
    std::vector<std::time_t> dates(unique_dates.begin(), unique_dates.end());
    return dates;
}

void PortfolioBuilder::buy(const YahooTimeseries& ticker_yt, double shares_amt, std::time_t date){
    struct AssetHolding* asset = this->get_asset(ticker_yt.get_ticker());
    double expense = shares_amt * ticker_yt.get_closes().get_ts_value(date);
    this->historical_cash_flow[date] -= expense;
    if (asset == nullptr){
        std::map<std::time_t, double> historical_cumulative_ticker_shares = {{date, shares_amt}};
        std::map<std::time_t, double> historical_cumulative_ticker_expenses = {{date, expense}};
        struct AssetHolding new_asset = {ticker_yt, historical_cumulative_ticker_shares, historical_cumulative_ticker_expenses};
        this->assets.emplace_back(new_asset);
    }
    else {
        asset->historical_cumulative_ticker_shares[date] = asset->historical_cumulative_ticker_shares.rbegin()->second + shares_amt;
        asset->historical_cumulative_ticker_expenses[date] = asset->historical_cumulative_ticker_expenses.rbegin()->second + expense;
    }
}

void PortfolioBuilder::sell(const YahooTimeseries& ticker_yt, double shares_amt, std::time_t date){
    struct AssetHolding* asset = this->get_asset(ticker_yt.get_ticker());
    if (asset != nullptr){
        double available_shares = asset->historical_cumulative_ticker_shares.rbegin()->second;
        double expense = shares_amt * ticker_yt.get_closes().get_ts_value(date);
        this->historical_cash_flow[date] += expense;
        if (shares_amt <= available_shares){
            asset->historical_cumulative_ticker_shares[date] = asset->historical_cumulative_ticker_shares.rbegin()->second - shares_amt;
            asset->historical_cumulative_ticker_expenses[date] = asset->historical_cumulative_ticker_expenses.rbegin()->second - expense;
        } 
        else
            fprintf(stderr, "not enough shares available to sell this volume of shares\n");
    }
    else
        fprintf(stderr, "ticker not in portfolio\n");
}

void PortfolioBuilder::set_portfolio_values(){
    std::map<std::time_t, double> ptf_values = this->get_ts_portfolio_values().get_ts_values();
    this->portfolio_values = ptf_values;
}

void PortfolioBuilder::save_portfolio(std::string filename) const{
    std::map<std::time_t, double> ptf_ts_values = this->get_portfolio_values();
    std::map<std::time_t, double> ptf_pls_ts_values = this->get_portfolio_profits_and_losses().get_ts_values();
    
    std::ofstream ptf_file("../strat_outputs/"+filename+".csv");
    double last_pls = 0.0;
    if (ptf_file.is_open()){
        ptf_file << "Date;Value;P&L" << std::endl;
        for (const auto& pair: ptf_ts_values){
            if (ptf_pls_ts_values[pair.first] == 0)
                ptf_file << unix_timestamp_to_date_string(pair.first) << ";" << pair.second << ";" << last_pls << std::endl;
            else
                ptf_file << unix_timestamp_to_date_string(pair.first) << ";" << pair.second << ";" << ptf_pls_ts_values[pair.first] << std::endl;
            last_pls = ptf_pls_ts_values[pair.first];
        }
        ptf_file.close();
    }
}

double PortfolioBuilder::get_ticker_value(std::string ticker, std::time_t date) const {
    const struct AssetHolding* asset = this->get_asset(ticker);
    
    if (asset == nullptr)
        return 0.0;
          
    double ticker_shares = 0.0;
    
    try {
        ticker_shares = asset->historical_cumulative_ticker_shares.at(date);
    } catch (const std::out_of_range& e) {  // Use out_of_range for map.at()
        auto it = asset->historical_cumulative_ticker_shares.lower_bound(date);
        
        if (it == asset->historical_cumulative_ticker_shares.begin() && it->first > date)
            return 0.0;
        if (it != asset->historical_cumulative_ticker_shares.begin())
            --it;
        ticker_shares = it->second;
    }

    return ticker_shares * asset->ticker_yt.get_closes().get_ts_value(date);
}

double PortfolioBuilder::get_ticker_expenses_value(std::string ticker, std::time_t date) const {
    const struct AssetHolding* asset = this->get_asset(ticker);
    
    if (asset == nullptr)
        return 0.0;

    double ticker_expenses = 0.0;
    try {
        ticker_expenses = asset->historical_cumulative_ticker_expenses.at(date);
    } catch (const std::out_of_range& e) {
        //std::cerr << "Ticker Expenses Value Exception: " << e.what() << " TICKER: " << ticker << " DATE: " << unix_timestamp_to_date_string(date) << std::endl;
        auto it = asset->historical_cumulative_ticker_expenses.lower_bound(date);
        
        if (it == asset->historical_cumulative_ticker_expenses.begin() && it->first > date)
            return 0.0;
        if (it != asset->historical_cumulative_ticker_expenses.begin()) {
            --it;
        }
        ticker_expenses = it->second;
    }

    return ticker_expenses;
}

double PortfolioBuilder::get_ticker_shares(std::string ticker, std::time_t date) const{
    const struct AssetHolding* asset = this->get_asset(ticker);
    
    if (asset == nullptr)
        return 0.0;

    double ticker_shares = 0.0;
    try {
        ticker_shares = asset->historical_cumulative_ticker_shares.at(date);
    } catch (const std::out_of_range& e) {
        //std::cerr << "Ticker Shares Exception: " << e.what() << " TICKER: " << ticker << " DATE: " << unix_timestamp_to_date_string(date) << std::endl;
        auto it = asset->historical_cumulative_ticker_shares.lower_bound(date);
        
        if (it == asset->historical_cumulative_ticker_shares.begin() && it->first > date)
            return 0.0;
        if (it != asset->historical_cumulative_ticker_shares.begin()) {
            --it;
        }
        ticker_shares = it->second;
    }

    return ticker_shares;
}

double PortfolioBuilder::get_portfolio_value(std::time_t date) const{
    double global_value = 0.0;
    for (auto& asset : this->assets){
        global_value += this->get_ticker_value(asset.ticker_yt.get_ticker(), date);
    }
    return global_value;
}

std::map<std::string, double> PortfolioBuilder::get_portfolio_percentage_allocations(std::time_t date) const{
    std::map<std::string, double> assets_pct_value_map;
    if (this->assets.empty())
        return assets_pct_value_map;
        
    std::vector<std::string> assets;
    std::vector<double> assets_pct_value;
    double ptf_value = 0.0;
    double asset_value;
    for (auto& asset : this->assets){
        std::string ticker = asset.ticker_yt.get_ticker();
        asset_value = this->get_ticker_value(ticker, date);
        ptf_value += asset_value;
        assets.push_back(ticker);
        assets_pct_value.push_back(asset_value);
    }

    std::transform(assets_pct_value.begin(), assets_pct_value.end(), assets_pct_value.begin(),
                   [ptf_value](double element) { return element / ptf_value; });
    
    std::transform(assets.begin(), assets.end(), assets_pct_value.begin(), std::inserter(assets_pct_value_map, assets_pct_value_map.end()),
                   [](const std::string& key, double value) { return std::make_pair(key, value); });
    return assets_pct_value_map;
}

std::map<std::time_t, double> PortfolioBuilder::get_portfolio_values() const{
    return this->portfolio_values;
}

Timeseries PortfolioBuilder::get_ticker_values(std::string ticker) const{
    const struct AssetHolding* asset = this->get_asset(ticker);
    if (asset == nullptr)
        return Timeseries({}, {0.0});
    
    std::vector<double> ticker_values;
    std::vector<std::time_t> dates;
    double ticker_value;
    for (const auto& dt : asset->ticker_yt.get_dates()){
            ticker_value = this->get_ticker_value(ticker, dt);
            ticker_values.push_back(ticker_value);
            dates.push_back(dt);
    }
    return Timeseries(dates, ticker_values);
}

Timeseries PortfolioBuilder::get_ts_portfolio_values() const{
    std::vector<std::time_t> ptf_dates = this->get_unique_portfolio_dates();
    std::vector<double> ptf_values;
    for (const auto& date: ptf_dates)
        ptf_values.push_back(this->get_portfolio_value(date));

    return Timeseries(ptf_dates, ptf_values);
}

Timeseries PortfolioBuilder::get_ticker_profits_and_losses(std::string ticker) const{
    const struct AssetHolding* asset = this->get_asset(ticker);
    if (asset == nullptr)
        return Timeseries({},{0.0});

    std::vector<std::time_t> ticker_dates = asset->ticker_yt.get_dates();
    std::map<std::time_t, double> ticker_values = this->get_ticker_values(ticker).get_ts_values();

    std::vector<std::time_t> dates;
    std::vector<double> ticker_pl_values;

    double acc_expenses_value = 0.0;
    double ticker_value;
    double close_value;

    for (const auto& dt : ticker_dates){ 
        //if (ticker_values[dt] - this->get_ticker_expenses_value(ticker, dt) == 0)
        //td::cout << unix_timestamp_to_date_string(dt) << " TICKER VALUE: " << ticker_values[dt] << " TICKER EXPENSE VALUE: " << this->get_ticker_expenses_value(ticker, dt) << std::endl;
        ticker_pl_values.push_back(ticker_values[dt] - this->get_ticker_expenses_value(ticker, dt));
        dates.push_back(dt);
    }
    return Timeseries(dates, ticker_pl_values);
}

Timeseries PortfolioBuilder::get_portfolio_profits_and_losses() const{
    if (this->assets.empty())
        return Timeseries({},{0.0});

    std::vector<double> pl_values;
    std::vector<std::map<std::time_t, double>> tickers_pl_ts_values;
    std::vector<std::time_t> unique_dates = this->get_unique_portfolio_dates();
    for (auto& asset : this->assets)
        tickers_pl_ts_values.push_back(this->get_ticker_profits_and_losses(asset.ticker_yt.get_ticker()).get_ts_values());

    for (const auto& dt : unique_dates){
        double date_pl_value = 0.0;
        for (auto& asset_pl_ts_value : tickers_pl_ts_values)
            date_pl_value += asset_pl_ts_value[dt];
        pl_values.push_back(date_pl_value);
    }
    return Timeseries(unique_dates, pl_values);
}

PortfolioBuilder::PortfolioBuilder()
: assets({}){}

PortfolioBuilder::~PortfolioBuilder(){}