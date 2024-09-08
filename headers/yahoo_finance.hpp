#ifndef YAHOO_FINANCE
#define YAHOO_FINANCE

#include <string>
#include <vector>
#include <ctime>
#include "./yahoo_timeseries.hpp"

class YahooFinance {

public:

    YahooFinance(const std::vector<std::string>& ticker_list, std::string start_date, std::string end_date, std::string freq);
    std::vector<YahooTimeseries> get_tickers_ts_data() const;
    void print_tickers_ts_data(const std::vector<YahooTimeseries>& tickers_st_data) const;
    ~YahooFinance();

private:

    std::vector<std::string> ticker_list; // list of tickers
    std::string start_date;
    std::string end_date;
    std::string freq;

    std::vector<std::string> get_tickers_str_data() const; // list of ticker's data
    void print_tickers_str_data(const std::vector<std::string>& tickers_data) const;

};

#endif