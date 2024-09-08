#include "../headers/yahoo_timeseries.hpp"
#include "../headers/yahoo_utils.hpp"
#include <cassert>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <iostream>

Timeseries::Timeseries():dates({}), values({}), ts_values({}){}

Timeseries::Timeseries(const std::vector<std::time_t>& dates, const std::vector<double>& values):dates(dates), values(values){
    assert(dates.size() == values.size()  && "dates and values passed must have the same length");
    for (size_t i = 0; i < dates.size(); ++i) {
        this->ts_values[dates[i]] = values[i];
    }
}

Timeseries::Timeseries(const std::map<std::time_t, double>& map_values): ts_values(map_values){
    size_t idx = 0;
    std::vector<std::time_t> dates(map_values.size());
    std::vector<double> values(map_values.size());
    for (const auto& pair: map_values){
        dates[idx] = pair.first;
        values[idx] = pair.second;
        idx++;
    }
    this->dates = dates;
    this->values = values;
}

bool Timeseries::operator==(const Timeseries& other) const {
    return dates == other.dates && vectors_almost_equal(values, other.values, 1e-3);
}

std::map<std::time_t, double> Timeseries::get_ts_values() const{
   return this->ts_values;
}

double Timeseries::get_ts_value(std::time_t date) const{
    double ts_value = 0.0;
    try {
        ts_value = this->ts_values.at(date);
    } catch (const std::out_of_range& e) {
        auto it = this->ts_values.lower_bound(date);
        if (it == this->ts_values.begin() && it->first > date)
            return ts_value;
        if (it != this->ts_values.begin())
            it--;
        //std::cerr << "TS Value Exception: " << e.what() << std::endl;

        if (it != this->ts_values.end())
            ts_value = it->second;
        else 
            std::cerr << "No valid date found. Defaulting to 0.0." << std::endl;
    }
    return ts_value;
}

std::vector<double> Timeseries::get_simple_moving_averages(size_t window_size) const{
    assert(this->values.size() >= window_size && "Error: Window size can't exceed the timeseries size\n");
    std::vector<double> averages(this->values.size() - window_size + 1);
    double sum = std::accumulate(this->values.begin(), this->values.begin() + window_size, 0.0);
    averages[0] = sum / window_size;
    for (size_t i = window_size; i < this->values.size(); ++i){
        sum += this->values[i] - this->values[i-window_size];
        averages[i - window_size + 1] = sum / window_size;
    }
    return averages;
}

std::map<std::time_t, double> Timeseries::get_ts_simple_moving_averages(size_t window_size) const{
    assert(this->values.size() >= window_size && "Error: Window size can't exceed the timeseries size\n");
    std::map<std::time_t, double> sma;
    double sum = std::accumulate(this->values.begin(), this->values.begin() + window_size, 0.0);
    sma[this->dates[window_size]] = sum / window_size;
    for (size_t i = window_size; i < this->values.size() - 1; ++i){
        sum += this->values[i] - this->values[i-window_size];
        sma[this->dates[i + 1]] = sum / window_size;
    }
    return sma;
}

std::vector<double> Timeseries::pget_simple_moving_averages(size_t window_size) const{
    assert(this->values.size() >= window_size && "Error: Window size can't exceed the timeseries size\n");
    std::vector<double> averages(this->values.size() - window_size + 1);
    for (size_t i=0; i < this->values.size() - window_size + 1; ++i){
        averages[i] = std::accumulate(this->values.begin() + i, this->values.begin() + i + window_size, 0.0) / window_size;
    }
    return averages;
}

std::vector<double> Timeseries::get_exponential_moving_averages(size_t window_size) const{
    assert(this->values.size() >= window_size && "Error: Window size can't exceed the timeseries size\n");
    std::vector<double> emas(this->values.size() - window_size + 1);
    emas[0] = std::accumulate(this->values.begin(), this->values.begin() + window_size, 0.0) / window_size;
    double alpha = 2.0 / (window_size + 1);
    for (size_t i=1; i < this->values.size() - window_size + 1; ++i){
        emas[i] = (this->values[window_size + i - 1] * alpha) + (emas[i-1] * (1 - alpha));
    }
    return emas;
}

std::map<std::time_t, double> Timeseries::get_ts_exponential_moving_averages(size_t window_size) const{
    assert(this->values.size() >= window_size && "Error: Window size can't exceed the timeseries size\n");
    std::map<std::time_t, double> emas;
    emas[this->dates[window_size]] = std::accumulate(this->values.begin(), this->values.begin() + window_size, 0.0) / window_size;
    double alpha = 2.0 / (window_size + 1);
    for (size_t i = 1; i < this->values.size() - window_size; ++i){
        emas[this->dates[i + window_size]] = (this->values[window_size + i - 1] * alpha) + (emas[this->dates[i + window_size - 1]] * (1 - alpha));
    }
    return emas;
}

std::vector<double> Timeseries::get_maximum_drawdowns(size_t window_size) const{
    assert(this->values.size() >= window_size && "Error: Window size can't exceed the timeseries size\n");
    assert(this->values.size() > 1 && "Error: Timeseries must contains at least 2 elements\n");
    std::vector<double> max_drawdowns(this->values.size() - window_size + 1);
    for (size_t i=0; i < this->values.size() - window_size + 1; ++i){
        double max_drawdown = 0.0;
        for (size_t j=0; j < window_size - 1; ++j){
            double diff = this->values[i + j] - *std::min_element(this->values.begin() + i + j + 1, this->values.begin() + i + window_size);
            if (diff > max_drawdown){
                max_drawdown = diff;
            }
        }
        max_drawdowns[i] = max_drawdown;
    }
    return max_drawdowns;
}

std::map<std::time_t, double> Timeseries::get_ts_maximum_drawdowns(size_t window_size) const{
    assert(this->values.size() >= window_size && "Error: Window size can't exceed the timeseries size\n");
    assert(this->values.size() > 1 && "Error: Timeseries must contains at least 2 elements\n");
    std::map<std::time_t, double> max_drawdowns;
    for (size_t i=0; i < this->values.size() - window_size; ++i){
        double max_drawdown = 0.0;
        for (size_t j=0; j < window_size - 1; ++j){
            double diff = this->values[i + j] - *std::min_element(this->values.begin() + i + j + 1, this->values.begin() + i + window_size);
            if (diff > max_drawdown){
                max_drawdown = diff;
            }
        }
        max_drawdowns[this->dates[i + window_size]] = max_drawdown;
    }
    return max_drawdowns;
}

std::vector<double> Timeseries::get_pct_changes() const{
    assert(this->values.size() > 1 && "Error: Timeseries must contains at least 2 elements\n");
    std::vector<double> pct_changes(this->values.size() - 1);
    for (size_t i=1; i < this->values.size(); ++i){
        pct_changes[i-1] = (this->values[i] - this->values[i-1]) / this->values[i-1];
    }
    return pct_changes;
}

std::map<std::time_t, double> Timeseries::get_ts_pct_changes() const{
    assert(this->values.size() > 2  && "Error: Timeseries must contains at least 3 elements for getting the pct change between d-2 and d-1 at date d\n");
    std::map<std::time_t, double> pct_changes;
    for (size_t i=1; i < this->values.size() - 1; ++i){
        pct_changes[this->dates[i+1]] = (this->values[i] - this->values[i-1]) / this->values[i-1];
    }
    return pct_changes;
}

std::vector<double> Timeseries::get_log_returns() const{
    assert(this->values.size() > 1 && "Error: Timeseries must contains at least 2 elements\n");
    std::vector<double> log_returns(this->values.size() - 1);
    for (size_t i=1; i < this->values.size(); ++i){
        log_returns[i-1] = std::log(this->values[i] / this->values[i-1]);
    }
    return log_returns;
}

std::map<std::time_t, double> Timeseries::get_ts_log_returns() const{
    assert(this->values.size() > 2 && "Error: Timeseries must contains at least 3 elements for getting the log change between d-2 and d-1 at date d\n");
    std::map<std::time_t, double> log_returns;
    for (size_t i=1; i < this->values.size(); ++i){
        log_returns[this->dates[i+1]] = std::log(this->values[i] / this->values[i-1]);
    }
    return log_returns;
}

std::vector<double> Timeseries::get_volatilities(size_t window_size) const{
    assert(window_size > 1 && "Error: Window size must be 2 at least\n");
    assert(this->values.size() >= window_size && "Error: Window size can't exceed the timeseries size\n");
    std::vector<double> volatilities(this->values.size() - window_size + 1);
    std::vector<double> smas = this->get_simple_moving_averages(window_size);
    for (size_t i=0; i < volatilities.size(); ++i){
        double sum_squared_diff = 0.0;
        for (size_t j=0; j<window_size; ++j){
            sum_squared_diff += (this->values[i + j] - smas[i]) * (this->values[i + j] - smas[i]);
        }
        volatilities[i] = std::sqrt(sum_squared_diff / window_size);
    }
    return volatilities;
}

std::map<std::time_t, double> Timeseries::get_ts_volatilities(size_t window_size) const{
    assert(window_size > 1 && "Error: Window size must be 2 at least\n");
    assert(this->values.size() > window_size && "Error: Window size must be below the timeseries size to get the volatily of previous dates d-window_size,...d-1 at date d\n");
    std::map<std::time_t, double> volatilities;
    std::vector<double> smas = this->get_simple_moving_averages(window_size);
    for (size_t i=0; i < smas.size() - 1; ++i){
        double sum_squared_diff = 0.0;
        for (size_t j=0; j<window_size; ++j){
            sum_squared_diff += (this->values[i + j] - smas[i]) * (this->values[i + j] - smas[i]);
        }
        volatilities[this->dates[i + window_size]] = std::sqrt(sum_squared_diff / window_size);
    }
    return volatilities;
}

std::vector<double> Timeseries::get_rsis(size_t window_size) const{
    assert(window_size > 1 && "Error: Window size must be 2 at least\n");
    assert(this->values.size() >= window_size && "Error: Window size can't exceed the timeseries size\n");
    std::vector<double> rsis(this->values.size() - window_size);
    double past_avg_gains = 0.0;
    double past_avg_losses = 0.0;
    for (size_t i=0; i < rsis.size(); ++i){
        std::vector<double> gains(rsis.size());
        std::vector<double> losses(rsis.size());
        double avg_gains = 0.0;
        double avg_losses = 0.0;
        double change = 0.0;
        double rsi = 0.0;
        for (size_t j=1; j<window_size + 1; ++j){
            change = this->values[i + j] - this->values[i + j - 1];
            if (change > 0) 
                avg_gains += change / window_size;
            else
                avg_losses -= change / window_size;
        }
        if (i == 0){
            past_avg_gains = avg_gains;
            past_avg_losses = avg_losses;
        }
        else {
            change = this->values[i + window_size] - this->values[i + window_size - 1];
            if (change > 0) {
                past_avg_gains = (past_avg_gains * (window_size - 1) + change) / window_size;
                past_avg_losses = past_avg_losses * (window_size - 1) / window_size;
            }
            else {
                past_avg_gains = past_avg_gains * (window_size - 1) / window_size;
                past_avg_losses = (past_avg_losses * (window_size - 1) - change) / window_size;
            }
        }
        rsi = past_avg_gains / past_avg_losses;
        rsis[i] = 100 -  (100 / (1 + rsi));
    }
    return rsis;
}

std::map<std::time_t, double> Timeseries::get_ts_rsis(size_t window_size) const{
    assert(window_size > 1 && "Error: Window size must be 2 at least\n");
    assert(this->values.size() > window_size && "Error: Window size must be below the timeseries size to get the rsi from d-window_size, ... d-1 at date d\n");
    std::map<std::time_t, double> rsis;
    double past_avg_gains = 0.0;
    double past_avg_losses = 0.0;
    double rsi_size = this->values.size() - window_size;
    for (size_t i=0; i < rsi_size; ++i){
        std::vector<double> gains(rsi_size);
        std::vector<double> losses(rsi_size);
        double avg_gains = 0.0;
        double avg_losses = 0.0;
        double change = 0.0;
        double rsi = 0.0;
        for (size_t j=1; j<window_size + 1; ++j){
            change = this->values[i + j] - this->values[i + j - 1];
            if (change > 0) 
                avg_gains += change / window_size;
            else
                avg_losses -= change / window_size;
        }
        if (i == 0){
            past_avg_gains = avg_gains;
            past_avg_losses = avg_losses;
        }
        else {
            change = this->values[i + window_size] - this->values[i + window_size - 1];
            if (change > 0) {
                past_avg_gains = (past_avg_gains * (window_size - 1) + change) / window_size;
                past_avg_losses = past_avg_losses * (window_size - 1) / window_size;
            }
            else {
                past_avg_gains = past_avg_gains * (window_size - 1) / window_size;
                past_avg_losses = (past_avg_losses * (window_size - 1) - change) / window_size;
            }
        }
        rsi = past_avg_gains / past_avg_losses;
        rsis[this->dates[i+window_size]] = 100 -  (100 / (1 + rsi));
    }
    return rsis;
}

Timeseries::~Timeseries(){};

YahooTimeseries::YahooTimeseries(const std::string ticker, const std::vector<std::time_t> dates, const std::vector<double> opens, const std::vector<double> lows, const std::vector<double> highs, const std::vector<double> closes, const std::vector<double> adjcloses)
:ticker(ticker), dates(dates), opens(dates, opens), lows(dates, lows), highs(dates, highs), closes(dates, closes), adjcloses(dates, adjcloses), dividends(Timeseries())
{
    assert(dates.size() == opens.size() && opens.size() == lows.size() && lows.size() == highs.size() && highs.size() == closes.size() && closes.size() == adjcloses.size() && "all vectors must be of the same length");
}

YahooTimeseries::YahooTimeseries(const std::string ticker, const std::vector<std::time_t> dates, const std::vector<double> opens, const std::vector<double> lows, const std::vector<double> highs, const std::vector<double> closes, const std::vector<double> adjcloses, const std::map<std::time_t, double> dividend_map)
:ticker(ticker), dates(dates), opens(dates, opens), lows(dates, lows), highs(dates, highs), closes(dates, closes), adjcloses(dates, adjcloses), dividends(Timeseries(dividend_map))
{
    assert(dates.size() == opens.size() && opens.size() == lows.size() && lows.size() == highs.size() && highs.size() == closes.size() && closes.size() == adjcloses.size() && "all vectors must be of the same length");
}

std::string YahooTimeseries::get_ticker() const{
    return this->ticker;
}

const std::vector<std::time_t>& YahooTimeseries::get_dates() const{
    return this->dates;
}

const Timeseries& YahooTimeseries::get_opens() const{
    return this->opens;
}

const Timeseries& YahooTimeseries::get_lows() const{
    return this->lows;
}

const Timeseries& YahooTimeseries::get_highs() const{
    return this->highs;
}

const Timeseries& YahooTimeseries::get_closes() const{
    return this->closes;
}

const Timeseries& YahooTimeseries::get_adjcloses() const{
    return this->adjcloses;
}

const Timeseries& YahooTimeseries::get_dividends() const{
    return this->dividends;
}

Timeseries YahooTimeseries::get_open_close_spreads() const{
    std::vector<std::time_t> dates = this->opens.dates;
    std::vector<double> spread(dates.size());
    std::vector<double> opens = this->opens.values;
    std::vector<double> closes =  this->closes.values;
    for (size_t i=0; i < spread.size(); ++i)
        spread[i] = closes[i] - opens[i];
    return Timeseries(dates, spread);
}

Timeseries YahooTimeseries::get_high_low_spreads() const{
    std::vector<std::time_t> dates = this->lows.dates;
    std::vector<double> spread(dates.size());
    std::vector<double> highs = this->highs.values;
    std::vector<double> lows = this->lows.values;
    for (size_t i=0; i < spread.size(); ++i)
        spread[i] = highs[i] - lows[i];
    return Timeseries(dates, spread);
}

YahooTimeseries::~YahooTimeseries(){};