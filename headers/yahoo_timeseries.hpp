#ifndef YAHOO_TIMESERIES
#define YAHOO_TIMESERIES

#include <string>
#include <vector>
#include <ctime>
#include <map>

class Timeseries {
public:
    Timeseries();
    Timeseries(const std::vector<std::time_t>& dates, const std::vector<double>& values);
    Timeseries(const std::map<std::time_t, double>& map_values);
    bool operator==(const Timeseries& other) const;
    
    std::map<std::time_t, double> get_ts_values() const;
    double get_ts_value(std::time_t date) const;

    std::vector<double> get_simple_moving_averages(size_t window_size) const;
    std::vector<double> pget_simple_moving_averages(size_t window_size) const; // Parallel Algorithm Version
    std::vector<double> get_exponential_moving_averages(size_t window_size) const;
    std::vector<double> get_maximum_drawdowns(size_t window_size) const;
    std::vector<double> get_pct_changes() const;
    std::vector<double> get_log_returns() const;
    std::vector<double> get_volatilities(size_t window_size) const;
    std::vector<double> get_rsis(size_t window_size) const;

    std::map<std::time_t, double> get_ts_simple_moving_averages(size_t window_size) const;
    std::map<std::time_t, double> get_ts_exponential_moving_averages(size_t window_size) const;
    std::map<std::time_t, double> get_ts_maximum_drawdowns(size_t window_size) const;
    std::map<std::time_t, double> get_ts_pct_changes() const;
    std::map<std::time_t, double> get_ts_log_returns() const;
    std::map<std::time_t, double> get_ts_volatilities(size_t window_size) const;
    std::map<std::time_t, double> get_ts_rsis(size_t window_size) const;
    ~Timeseries();
private:
    std::vector<std::time_t> dates;
    std::vector<double> values;
    std::map<std::time_t, double> ts_values;

friend class YahooTimeseries;
};

class YahooTimeseries {
public:
    YahooTimeseries(const std::string ticker,
                    const std::vector<std::time_t> dates,
                    const std::vector<double> opens, 
                    const std::vector<double> lows, 
                    const std::vector<double> highs, 
                    const std::vector<double> closes,
                    const std::vector<double> adjcloses,
                    const std::map<std::time_t, double> dividends);
     YahooTimeseries(const std::string ticker,
                    const std::vector<std::time_t> dates,
                    const std::vector<double> opens, 
                    const std::vector<double> lows, 
                    const std::vector<double> highs, 
                    const std::vector<double> closes,
                    const std::vector<double> adjcloses);
    std::string get_ticker() const;
    const std::vector<std::time_t>& get_dates() const;
    const Timeseries& get_opens() const;
    const Timeseries& get_lows() const;
    const Timeseries& get_highs() const;
    const Timeseries& get_closes() const;
    const Timeseries& get_adjcloses() const;
    const Timeseries& get_dividends() const;
    
    Timeseries get_open_close_spreads() const;
    Timeseries get_high_low_spreads() const;
    ~YahooTimeseries();


private:
    std::string ticker;
    std::vector<std::time_t> dates;
    Timeseries opens;
    Timeseries lows;
    Timeseries highs;
    Timeseries closes;
    Timeseries adjcloses;
    Timeseries dividends;
};

#endif