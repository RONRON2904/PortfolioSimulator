#include "../headers/portfolio_builder.hpp"
#include "../headers/yahoo_utils.hpp"
#include <eigen3/Eigen/Dense>
#include <iostream>
#include <fstream>
#include <chrono>

struct AssetHolding *PortfolioBuilder::get_asset(std::string ticker)
{
    for (auto &asset : this->assets)
        if (asset.ticker_yt.get_ticker() == ticker)
            return &asset;
    return nullptr;
}

const struct AssetHolding *PortfolioBuilder::get_asset(std::string ticker) const
{
    for (auto &asset : this->assets)
        if (asset.ticker_yt.get_ticker() == ticker)
            return &asset;
    return nullptr;
}

const std::map<std::time_t, double> &PortfolioBuilder::get_portfolio_historical_cash_flow() const
{
    return this->historical_cash_flow;
}

std::vector<std::time_t> PortfolioBuilder::get_unique_portfolio_dates() const
{
    std::set<std::time_t> unique_dates;
    for (auto &asset : this->assets)
        for (const auto &ticker_date : asset.ticker_yt.get_dates())
            unique_dates.insert(ticker_date);
    std::vector<std::time_t> dates(unique_dates.begin(), unique_dates.end());
    return dates;
}

void PortfolioBuilder::deposit(double cash_amt, std::time_t date)
{
    if (this->historical_cash.empty()){
        this->historical_cash = {{date, cash_amt}};
        this->historical_cumulative_deposits = {{date, cash_amt}};
    }
    else {
        this->historical_cash[date] = this->historical_cash.rbegin()->second + cash_amt;
        this->historical_cumulative_deposits[date] = this->historical_cumulative_deposits.rbegin()->second + cash_amt;
    }
}

void PortfolioBuilder::receive_dividend(double cash_amt, std::time_t date){
    if (this->historical_cash.empty())
        this->historical_cash = {{date, cash_amt}};
    else
        this->historical_cash[date] = this->historical_cash.rbegin()->second + cash_amt;
}

void PortfolioBuilder::withdraw(double cash_amt, std::time_t date){
    if (this->get_cash_amount(date) >= cash_amt)
        this->historical_cash[date] = this->historical_cash.rbegin()->second - cash_amt;
}

void PortfolioBuilder::buy(const YahooTimeseries &ticker_yt, double shares_amt, std::time_t date)
{
    struct AssetHolding *asset = this->get_asset(ticker_yt.get_ticker());
    double expense = shares_amt * ticker_yt.get_closes().get_ts_value(date);
    if (this->get_cash_amount(date) - expense >= -0.001)
    {
        this->historical_cash_flow[date] -= expense;
        if (asset == nullptr)
        {
            std::map<std::time_t, double> historical_cumulative_ticker_shares = {{date, shares_amt}};
            std::map<std::time_t, double> historical_cumulative_ticker_expenses = {{date, expense}};
            struct AssetHolding new_asset = {ticker_yt, historical_cumulative_ticker_shares, historical_cumulative_ticker_expenses};
            this->assets.emplace_back(new_asset);
        }
        else
        {
            asset->historical_cumulative_ticker_shares[date] = asset->historical_cumulative_ticker_shares.rbegin()->second + shares_amt;
            asset->historical_cumulative_ticker_expenses[date] = asset->historical_cumulative_ticker_expenses.rbegin()->second + expense;
        }
        if (this->portfolio_total_shares.empty())
            this->portfolio_total_shares[date] = shares_amt;
        else
            this->portfolio_total_shares[date] = this->portfolio_total_shares.rbegin()->second + shares_amt;
        this->historical_cash[date] = this->historical_cash.rbegin()->second - expense;
    }
    
    else{
        fprintf(stderr, "not enough cash available to buy this volume of shares\n");
        std::cout << unix_timestamp_to_date_string(date) << std::endl;
        std::cout << ticker_yt.get_ticker() << std::endl;
        std::cout << expense << std::endl;
        std::cout << this->get_cash_amount(date) << std::endl;
    }
}

void PortfolioBuilder::sell(const YahooTimeseries &ticker_yt, double shares_amt, std::time_t date)
{
    struct AssetHolding *asset = this->get_asset(ticker_yt.get_ticker());
    if (asset != nullptr)
    {
        double available_shares = asset->historical_cumulative_ticker_shares.rbegin()->second;
        double expense = shares_amt * ticker_yt.get_closes().get_ts_value(date);
        this->historical_cash_flow[date] += expense;
        if (shares_amt <= available_shares)
        {
            asset->historical_cumulative_ticker_shares[date] = asset->historical_cumulative_ticker_shares.rbegin()->second - shares_amt;
            asset->historical_cumulative_ticker_expenses[date] = asset->historical_cumulative_ticker_expenses.rbegin()->second - expense;
            this->portfolio_total_shares[date] = this->portfolio_total_shares.rbegin()->second - shares_amt;
            this->historical_cash[date] = this->historical_cash.rbegin()->second + expense;
        }
        else
            fprintf(stderr, "not enough shares available to sell this volume of shares\n");
    }
    else
        fprintf(stderr, "ticker not in portfolio\n");
}

void PortfolioBuilder::set_portfolio_values_and_prices()
{
    std::map<std::time_t, double> ptf_values = this->get_ts_portfolio_values().get_ts_values();
    this->portfolio_values = ptf_values;

    for (const auto &pair : ptf_values)
    {
        double total_shares = this->get_portfolio_total_shares(pair.first);
        this->portfolio_prices[pair.first] = ptf_values[pair.first] / total_shares;
    }
}

void PortfolioBuilder::save_portfolio(std::string filename) const
{
    std::map<std::time_t, double> ptf_ts_values = this->get_portfolio_values();
    std::map<std::time_t, double> ptf_pls_ts_values = this->get_portfolio_profits_and_losses().get_ts_values();

    std::ofstream ptf_file("../strat_outputs/" + filename + ".csv");
    double last_pls = 0.0;
    if (ptf_file.is_open())
    {
        ptf_file << "Date;Value;P&L;Investments;Cash" << std::endl;
        for (const auto &pair : ptf_ts_values)
        {
            double cash = std::round(this->get_cash_amount(pair.first) * 100.0) / 100.0;
            if (ptf_pls_ts_values[pair.first] == 0)
                ptf_file << unix_timestamp_to_date_string(pair.first) << ";" << pair.second << ";" << last_pls << ";" << pair.second - last_pls << ";" << cash << std::endl;
            else{
                last_pls = ptf_pls_ts_values[pair.first];
                ptf_file << unix_timestamp_to_date_string(pair.first) << ";" << pair.second << ";" << ptf_pls_ts_values[pair.first] << ";" << pair.second - last_pls << ";" << cash << std::endl;
            }
        }
        ptf_file.close();
    }
}

double PortfolioBuilder::get_cash_amount(std::time_t date) const
{
    double cash_amount = 0.0;
    try
    {
        cash_amount = this->historical_cash.at(date);
    }
    catch (const std::out_of_range &e)
    { // Use out_of_range for map.at()
        auto it = this->historical_cash.lower_bound(date);

        if (it == this->historical_cash.begin() && it->first > date)
            return 0.0;
        if (it != this->historical_cash.begin())
            --it;
        cash_amount = it->second;
    }
    return cash_amount;
}


double PortfolioBuilder::get_ticker_value(std::string ticker, std::time_t date) const
{
    const struct AssetHolding *asset = this->get_asset(ticker);

    if (asset == nullptr)
        return 0.0;

    double ticker_shares = 0.0;

    try
    {
        ticker_shares = asset->historical_cumulative_ticker_shares.at(date);
    }
    catch (const std::out_of_range &e)
    { // Use out_of_range for map.at()
        auto it = asset->historical_cumulative_ticker_shares.lower_bound(date);

        if (it == asset->historical_cumulative_ticker_shares.begin() && it->first > date)
            return 0.0;
        if (it != asset->historical_cumulative_ticker_shares.begin())
            --it;
        ticker_shares = it->second;
    }
    return round(ticker_shares * asset->ticker_yt.get_closes().get_ts_value(date) * 100.0) / 100.0;
}

double PortfolioBuilder::get_ticker_expenses_value(std::string ticker, std::time_t date) const
{
    const struct AssetHolding *asset = this->get_asset(ticker);

    if (asset == nullptr)
        return 0.0;

    double ticker_expenses = 0.0;
    try
    {
        ticker_expenses = asset->historical_cumulative_ticker_expenses.at(date);
    }
    catch (const std::out_of_range &e)
    {
        // std::cerr << "Ticker Expenses Value Exception: " << e.what() << " TICKER: " << ticker << " DATE: " << unix_timestamp_to_date_string(date) << std::endl;
        auto it = asset->historical_cumulative_ticker_expenses.lower_bound(date);

        if (it == asset->historical_cumulative_ticker_expenses.begin() && it->first > date)
            return 0.0;
        if (it != asset->historical_cumulative_ticker_expenses.begin())
        {
            --it;
        }
        ticker_expenses = it->second;
    }

    return ticker_expenses;
}

double PortfolioBuilder::get_ticker_shares(std::string ticker, std::time_t date) const
{
    const struct AssetHolding *asset = this->get_asset(ticker);

    if (asset == nullptr)
        return 0.0;

    double ticker_shares = 0.0;
    try
    {
        ticker_shares = asset->historical_cumulative_ticker_shares.at(date);
    }
    catch (const std::out_of_range &e)
    {
        // std::cerr << "Ticker Shares Exception: " << e.what() << " TICKER: " << ticker << " DATE: " << unix_timestamp_to_date_string(date) << std::endl;
        auto it = asset->historical_cumulative_ticker_shares.lower_bound(date);

        if (it == asset->historical_cumulative_ticker_shares.begin() && it->first > date)
            return 0.0;
        if (it != asset->historical_cumulative_ticker_shares.begin())
        {
            --it;
        }
        ticker_shares = it->second;
    }

    return ticker_shares;
}

double PortfolioBuilder::get_portfolio_value(std::time_t date) const
{
    double global_value = this->get_cash_amount(date);
    for (auto &asset : this->assets)
    {
        global_value += this->get_ticker_value(asset.ticker_yt.get_ticker(), date);
    }
    return std::round(global_value * 100.0) / 100.0;
}

double PortfolioBuilder::get_portfolio_cumulative_deposit(std::time_t date) const
{
    double cumulative_deposit = 0.0;
    try
    {
        cumulative_deposit = this->historical_cumulative_deposits.at(date);
    }
    catch(const std::exception& e)
    {
        auto it = this->historical_cumulative_deposits.lower_bound(date);

        if (it == this->historical_cumulative_deposits.begin() && it->first > date)
            return 0.0;
        if (it != this->historical_cumulative_deposits.begin())
            --it;
        cumulative_deposit = it->second;
    }
    
    return cumulative_deposit;
}

double PortfolioBuilder::get_portfolio_total_shares(std::time_t date) const
{
    double total_shares = 0.0;
    try
    {
        total_shares = this->portfolio_total_shares.at(date);
    }
    catch (const std::out_of_range &e)
    {
        // std::cerr << "Ticker Shares Exception: " << e.what() << " TICKER: " << ticker << " DATE: " << unix_timestamp_to_date_string(date) << std::endl;
        auto it = this->portfolio_total_shares.lower_bound(date);

        if (it == this->portfolio_total_shares.begin() && it->first > date)
            return 0.0;
        if (it != this->portfolio_total_shares.begin())
        {
            --it;
        }
        total_shares = it->second;
    }

    return total_shares;
}

std::map<std::string, double> PortfolioBuilder::get_portfolio_percentage_allocations(std::time_t date) const
{
    std::map<std::string, double> assets_pct_value_map;
    if (this->assets.empty() && this->historical_cash.empty())
        return assets_pct_value_map;

    std::vector<std::string> assets;
    std::vector<double> assets_pct_value;
    double cash_amt = this->get_cash_amount(date);
    double ptf_value = cash_amt; //0.0;
    double asset_value;
    for (auto &asset : this->assets)
    {
        std::string ticker = asset.ticker_yt.get_ticker();
        asset_value = this->get_ticker_value(ticker, date);
        ptf_value += asset_value;
        assets.push_back(ticker);
        assets_pct_value.push_back(asset_value);
    }

    std::transform(assets_pct_value.begin(), assets_pct_value.end(), assets_pct_value.begin(),
                   [ptf_value](double element)
                   { return element / ptf_value; });

    std::transform(assets.begin(), assets.end(), assets_pct_value.begin(), std::inserter(assets_pct_value_map, assets_pct_value_map.end()),
                   [](const std::string &key, double value)
                   { return std::make_pair(key, value); });

    assets_pct_value_map["Cash"] = cash_amt / ptf_value;
    return assets_pct_value_map;
}

std::map<std::time_t, double> PortfolioBuilder::get_portfolio_values() const
{
    return this->portfolio_values;
}

std::map<std::time_t, double> PortfolioBuilder::get_portfolio_cumulative_deposits() const
{
    return this->historical_cumulative_deposits;
}

std::map<std::time_t, double> PortfolioBuilder::get_portfolio_historical_cash() const
{
    return this->historical_cash;
}

std::map<time_t, std::vector<double>> PortfolioBuilder::get_portfolio_values_and_pls() const {
    std::map<std::time_t, double> ptf_values = this->get_ts_portfolio_values().get_ts_values();
    std::map<std::time_t, double> ptf_pls_ts_values = this->get_portfolio_profits_and_losses().get_ts_values();
    std::map<std::time_t, double> ptf_ath_pct_change_values = this->get_ts_portfolio_values().get_ts_pct_changes_since_last_max();
    std::map<time_t, std::vector<double>> values_pls;
    for (const auto &pair : ptf_values)
    {
        values_pls[pair.first] = {ptf_values[pair.first], ptf_pls_ts_values[pair.first], ptf_ath_pct_change_values[pair.first]};
    }
    return values_pls;
}

Timeseries PortfolioBuilder::get_ticker_values(std::string ticker) const
{
    const struct AssetHolding *asset = this->get_asset(ticker);
    if (asset == nullptr)
        return Timeseries({}, {0.0});

    std::vector<double> ticker_values;
    std::vector<std::time_t> dates;
    double ticker_value;
    for (const auto &dt : asset->ticker_yt.get_dates())
    {
        ticker_value = this->get_ticker_value(ticker, dt);
        ticker_values.push_back(ticker_value);
        dates.push_back(dt);
    }
    return Timeseries(dates, ticker_values);
}

Timeseries PortfolioBuilder::get_ts_portfolio_values() const
{
    std::vector<std::time_t> ptf_dates = this->get_unique_portfolio_dates();
    std::vector<double> ptf_values;
    for (const auto &date : ptf_dates)
        ptf_values.push_back(this->get_portfolio_value(date));

    return Timeseries(ptf_dates, ptf_values);
}

Timeseries PortfolioBuilder::get_ts_portfolio_prices() const
{
    return Timeseries(this->portfolio_prices);
}

Timeseries PortfolioBuilder::get_ticker_profits_and_losses(std::string ticker) const
{
    const struct AssetHolding *asset = this->get_asset(ticker);
    if (asset == nullptr)
        return Timeseries({}, {0.0});

    std::vector<std::time_t> ticker_dates = asset->ticker_yt.get_dates();
    std::map<std::time_t, double> ticker_values = this->get_ticker_values(ticker).get_ts_values();

    std::vector<std::time_t> dates;
    std::vector<double> ticker_pl_values;

    double acc_expenses_value = 0.0;
    double ticker_value;
    double close_value;

    for (const auto &dt : ticker_dates)
    {
        ticker_pl_values.push_back(ticker_values[dt] - this->get_ticker_expenses_value(ticker, dt));
        dates.push_back(dt);
    }
    return Timeseries(dates, ticker_pl_values);
}

Timeseries PortfolioBuilder::get_portfolio_profits_and_losses() const
{
    if (this->assets.empty())
        return Timeseries({}, {0.0});

    std::vector<double> pl_values;
    std::vector<std::time_t> unique_dates = this->get_unique_portfolio_dates();

    for (const auto &dt : unique_dates)
    {
        double date_pl_value = this->get_portfolio_value(dt) - this->get_portfolio_cumulative_deposit(dt); 
        pl_values.push_back(std::round(date_pl_value * 100.0) / 100.0);
    }
    return Timeseries(unique_dates, pl_values);
}

PortfolioBuilder::PortfolioBuilder()
    : assets({}), portfolio_total_shares({}), historical_cash({}) {}

PortfolioBuilder::~PortfolioBuilder() {}