#ifndef STRATEGY_HPP
#define STRATEGY_HPP

#include "./portfolio_builder.hpp"
#include "./strategy_config.hpp"

class CustomStrategy
{
public:
    explicit CustomStrategy(StrategyConfig &config);
    void make_transactions(std::time_t date);
    std::map<time_t, std::vector<double>> run_strategy();

    const std::map<std::time_t, double> get_strategy_values() const;
    double get_strategy_total_returns() const;
    double get_strategy_extended_internal_return_rate(double tolerance, int max_iterations) const;
    double get_strategy_total_investments() const;
    double get_strategy_max_drawdown() const;
    void save_end_portfolio(std::string filename);

    const YahooTimeseries montecarlo_simulation(const std::vector<std::time_t> &future_dates);
    void run_montecarlo_simulations(size_t nb_simu);
    ~CustomStrategy();

private:
    PortfolioBuilder *ptf; // Try with PortfolioBuilder &ptf;
    StrategyConfig &config;

    void handle_recurrent_investment_parameters(std::time_t date);
    void apply_technical_indicators(std::time_t date);
    void handle_risk_parameters(std::time_t date);
    void rebalance_portfolio(std::time_t date);
};

class Strategy
{
public:
    explicit Strategy(const std::vector<YahooTimeseries> &tickers_yt, std::string strategy_name);
    virtual void make_transactions(std::time_t date) = 0;
    virtual void run_strategy();

    virtual const std::map<std::time_t, double> get_strategy_values() const;
    virtual double get_strategy_total_returns() const;
    virtual double get_strategy_extended_internal_return_rate(double tolerance, int max_iterations) const;
    virtual void save_end_portfolio();

    virtual const YahooTimeseries montecarlo_simulation(const std::vector<std::time_t> &future_dates);
    virtual void run_montecarlo_simulations(size_t nb_simu) = 0;
    virtual ~Strategy();

protected:
    std::string strategy_name;
    std::vector<YahooTimeseries> tickers_yt;
    PortfolioBuilder *ptf;
};

class DCA : public Strategy
{
public:
    DCA(const std::vector<YahooTimeseries> &tickers_yt,
        double starting_amount,
        double recurrent_investment_amount,
        const std::map<std::string, double> &assets_desired_pct_allocations,
        int rebalancing_freq,
        double rebalancing_threshold,
        std::string strategy_name);
    void rebalance_portfolio(std::time_t date);
    void make_transaction(const YahooTimeseries &ticker_yt, std::time_t date);
    virtual void make_transactions(std::time_t date) override;
    virtual void run_montecarlo_simulations(size_t nb_simu) override;

protected:
    double starting_amount;
    double recurrent_investment_amount;
    std::map<std::string, double> assets_desired_pct_allocations;
    std::map<std::string, double> assets_starting_amounts;
    double rebalancing_threshold;
    int rebalancing_freq;
    int last_rebalancing_nb_days;
    std::map<std::string, std::vector<std::time_t>> tickers_first_month_dates;
    std::map<std::string, std::vector<std::time_t>> tickers_last_month_dates;
};

class SmaOptimizedDCA : public DCA
{
public:
    SmaOptimizedDCA(const std::vector<YahooTimeseries> &tickers_yt,
                    double starting_amount,
                    double recurrent_investment_amount,
                    const std::map<std::string, double> &assets_desired_pct_allocations,
                    int rebalancing_freq,
                    double rebalancing_threshold,
                    int sma_window_size,
                    std::string strategy_name);
    void make_transaction(const YahooTimeseries &ticker_yt, std::time_t date, const Timeseries &simple_moving_avergages);
    virtual void make_transactions(std::time_t date) override;

private:
    std::map<std::string, double> current_tickers_remaining_investment_amount;
    std::map<std::string, std::vector<std::time_t>> tickers_last_month_dates;
    std::map<std::string, Timeseries> tickers_sma;
};

class LumpSum : public Strategy
{
public:
    LumpSum(const std::vector<YahooTimeseries> &tickers_yt,
            double initial_investment_amount,
            const std::map<std::string, double> &assets_desired_pct_allocations,
            int rebalancing_freq,
            double rebalancing_threshold,
            std::string strategy_name);
    void rebalance_portfolio(std::time_t date);
    void make_transaction(const YahooTimeseries &ticker_yt, std::time_t date);
    void make_transactions(std::time_t date) override;

private:
    double initial_investment_amount;
    std::map<std::string, double> assets_desired_pct_allocations;
    double rebalancing_threshold;
    int rebalancing_freq;
    int last_rebalancing_nb_days;
    std::map<std::string, std::time_t> tickers_first_date;
};

#endif