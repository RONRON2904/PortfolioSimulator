#ifndef YAHOO_UTILS
#define YAHOO_UTILS

#include <string>
#include <ctime>
#include <set>
#include <nlohmann/json.hpp>
#include "../headers/yahoo_finance.hpp"

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);
std::time_t date_string_to_unix_timestamp(std::string date_string);
std::string unix_timestamp_to_date_string(time_t ts_date);
std::vector<std::string> unix_timestamps_to_date_strings(const std::vector<time_t> ts_dates);
std::string get_ticker_str_data(std::string ticker, std::string start_date, std::string end_date, std::string freq);
void remove_null_values_indexes(std::vector<nlohmann::json>& json_list_values);
YahooTimeseries get_ticker_ts_data(std::string ticker_str_data);
std::vector<double> get_exponential_moving_average(std::vector<double> prices, double alpha);
size_t get_date_index(std::time_t date, std::vector<std::time_t> dates);
std::map<std::time_t, double> init_map(const YahooTimeseries &ticker_yt);
std::vector<std::time_t> generate_random_dates(size_t count, std::time_t start, std::time_t end);
std::vector<std::time_t> get_unique_dates(std::vector<YahooTimeseries> tickers_yt);
std::vector<std::time_t> extract_first_dates_of_each_month(const std::vector<std::time_t> &dates);
std::vector<std::time_t> extract_last_dates_of_each_month(const std::vector<std::time_t> &dates);
std::time_t generate_strategy_config_ym_date(int year, int month, int investment_montly_weeknum, int investment_week_day);
std::vector<std::time_t> extract_strategy_config_recurrent_investment_dates(const std::vector<std::time_t> &dates, int investment_nb_days_frequency, int investment_montly_weeknum, int investment_week_day);
bool almost_equal(double a, double b, double epsilon);
bool vectors_almost_equal(const std::vector<double> &v1, const std::vector<double> &v2, double epsilon);
double get_standard_deviation(const std::vector<double> &values);
bool contains_all_tickers_yt_vectors(const std::vector<YahooTimeseries> &all_tickers_yt, const std::vector<YahooTimeseries> &sub_tickers_yt);
nlohmann::json read_json_file(std::string json_filepath);
std::vector<std::string> parse_string_list(const std::string& input);
std::map<std::string, double> parse_string_map(const std::string& input);
std::vector<std::string> parse_string_array(const std::string& input);
std::set<std::string> flatten_to_set(const std::vector<std::vector<std::string>>& list_of_lists);
double get_closest_value(std::time_t date, const std::map<std::time_t, double>& values_map);

#endif