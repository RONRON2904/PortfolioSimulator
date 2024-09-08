#include "../headers/yahoo_utils.hpp"

#include <curl/curl.h>
#include <iostream>

YahooFinance::YahooFinance(const std::vector<std::string>& ticker_list, std::string start_date, std::string end_date, std::string freq)
: ticker_list(ticker_list), start_date(start_date), end_date(end_date), freq(freq){}

std::vector<YahooTimeseries> YahooFinance::get_tickers_ts_data() const{
    std::vector<YahooTimeseries> tickers_yt_data;
    for (size_t i = 0; i < this->ticker_list.size(); ++i){
        std::string ticker_str_data = get_ticker_str_data(this->ticker_list[i], this->start_date, this->end_date, this->freq);
        YahooTimeseries ticker_yt_data = get_ticker_ts_data(ticker_str_data);
        tickers_yt_data.emplace_back(ticker_yt_data);
    }
    return tickers_yt_data;
}

void YahooFinance::print_tickers_ts_data(const std::vector<YahooTimeseries>& tickers_yt_data) const{
    for (size_t i = 0; i < this->ticker_list.size(); ++i){
        std::vector<time_t> dates = tickers_yt_data[i].get_dates();
        std::vector<std::string> tickers_dates =  unix_timestamps_to_date_strings(dates);
        std::cout << "TICKER:" << tickers_yt_data[i].get_ticker() << std::endl;
        std::map<std::time_t, double> opens = tickers_yt_data[i].get_opens().get_ts_values();
        std::map<std::time_t, double> highs = tickers_yt_data[i].get_highs().get_ts_values();
        std::map<std::time_t, double> lows = tickers_yt_data[i].get_lows().get_ts_values();
        std::map<std::time_t, double> closes = tickers_yt_data[i].get_closes().get_ts_values();
        std::map<std::time_t, double> adjcloses = tickers_yt_data[i].get_adjcloses().get_ts_values();
        std::map<std::time_t, double> dividends = tickers_yt_data[i].get_dividends().get_ts_values();
        if (dividends.size() > 0){
            double dividend;
            for (size_t j = 0; j < tickers_dates.size(); ++j){
                std::time_t date = dates[j];
                if (dividends.find(date) != dividends.end())
                    dividend = dividends[date];
                else
                    dividend  = 0;
                std::cout << "DATE: " << tickers_dates[j] << " OPEN: " << opens[date] << " HIGH:" << highs[date] << " LOW: " << lows[date] << " CLOSE: " << closes[date] << " ADJCLOSE: " << adjcloses[date] << " DIVIDEND: " << dividend << std::endl;
            }
        }
        else {
            for (size_t j = 0; j < tickers_dates.size(); ++j){
                std::time_t date = dates[j];
                std::cout << "DATE: " << tickers_dates[j] << " OPEN: " << opens[date] << " HIGH:" << highs[date] << " LOW: " << lows[date] << " CLOSE: " << closes[date] << " ADJCLOSE: " << adjcloses[date] << std::endl;
            }
        }
    }
}

std::vector<std::string> YahooFinance::get_tickers_str_data() const{
    std::vector<std::string> tickers_str_data;
    for (size_t i = 0; i < this->ticker_list.size(); ++i){
        std::string ticker_str_data = get_ticker_str_data(this->ticker_list[i], this->start_date, this->end_date, this->freq);
        tickers_str_data.emplace_back(ticker_str_data);
    }
    return tickers_str_data;
}

void YahooFinance::print_tickers_str_data(const std::vector<std::string>& tickers_data) const{
    for (size_t i = 0; i < this->ticker_list.size(); ++i){
        std::cout << "Received data:\n" << tickers_data[i] << std::endl;
    }
}

YahooFinance::~YahooFinance(){}