#ifndef YAHOO_UTILS
#define YAHOO_UTILS

#include <string>
#include <ctime>
#include <set>
#include "../headers/yahoo_finance.hpp"
#include <eigen3/Eigen/Dense>

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
std::time_t date_string_to_unix_timestamp(std::string date_string);
std::string unix_timestamp_to_date_string(time_t ts_date);
std::vector<std::string> unix_timestamps_to_date_strings(const std::vector<time_t> ts_dates);
std::string get_ticker_str_data(std::string ticker, std::string start_date, std::string end_date, std::string freq);
YahooTimeseries get_ticker_ts_data(std::string ticker_str_data);
std::vector<double> get_exponential_moving_average(std::vector<double> prices, double alpha);
size_t get_date_index(std::time_t date, std::vector<std::time_t> dates);
std::map<std::time_t, double> init_map(const YahooTimeseries& ticker_yt);
std::vector<std::time_t> generate_random_dates(size_t count, std::time_t start, std::time_t end);
std::vector<std::time_t> get_unique_dates(std::vector<YahooTimeseries> tickers_yt);
std::vector<std::time_t> extract_first_dates_of_each_month(const std::vector<std::time_t>& dates);
std::vector<std::time_t> extract_last_dates_of_each_month(const std::vector<std::time_t>& dates);
bool almost_equal(double a, double b, double epsilon);
bool vectors_almost_equal(const std::vector<double>& v1, const std::vector<double>& v2, double epsilon);
double get_standard_deviation(const std::vector<double>& values);

#endif