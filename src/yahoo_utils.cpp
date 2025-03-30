#include "../headers/yahoo_utils.hpp"
#include <curl/curl.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <nlohmann/json.hpp>
#include <vector>
#include <random>
#include <fstream>

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp){
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::time_t date_string_to_unix_timestamp(std::string date_string){
    std::tm tm = {};
    std::istringstream ss(date_string);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    
    if (ss.fail()) {
        throw std::runtime_error("Failed to parse date string");
    }

    std::time_t time = std::mktime(&tm);
    if (time == -1) {
        throw std::runtime_error("Failed to convert to time_t");
    }

    return time;
}

std::string unix_timestamp_to_date_string(time_t ts_date) {
    struct tm tm;
    localtime_r(&ts_date, &tm);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");

    return oss.str();
}

std::vector<std::string> unix_timestamps_to_date_strings(std::vector<time_t> ts_dates) {
    std::vector<std::string> str_dates;

    for (size_t i = 0; i < ts_dates.size(); ++i) {
        // Convert time_t to tm structure
        struct tm tm;
        localtime_r(&ts_dates[i], &tm);

        // Create a string stream to format the date
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d");

        // Add the formatted date string to the vector
        str_dates.push_back(oss.str());
    }
    return str_dates;
}

bool is_first_day_of_month(std::time_t date){
    std::tm* time_info = std::localtime(&date);
    return (time_info->tm_mday == 1);
}

std::vector<std::time_t> get_unique_dates(std::vector<YahooTimeseries> tickers_yt){
    std::set<std::time_t> unique_dates;
     for (auto& ticker_yt : tickers_yt)
         for (const auto& ticker_date : ticker_yt.get_dates())
            unique_dates.insert(ticker_date);
    std::vector<std::time_t> new_dates(unique_dates.begin(), unique_dates.end());
    std::sort(new_dates.begin(), new_dates.end());
    return new_dates;
}

std::string get_ticker_str_data(std::string ticker, std::string start_date, std::string end_date, std::string freq){
    std::time_t period1 = date_string_to_unix_timestamp(start_date);
    std::time_t period2 = date_string_to_unix_timestamp(end_date);
    
    std::stringstream ss1; 
    ss1 << period1; 
    std::stringstream ss2; 
    ss2 << period2;

    std::string url = "https://query2.finance.yahoo.com/v8/finance/chart/"
                    + ticker
                    + "?period1=" + ss1.str()
                    + "&period2=" + ss2.str()
                    + "&interval=" + freq
                    + "&events=div";

    CURL* curl = curl_easy_init();
    std::string response_buffer;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.2; .NET CLR 1.0.3705;)");

        // Write result into the buffer
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);


        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

        // Cleanup
        curl_easy_cleanup(curl);
    }

    return response_buffer;
}

void remove_null_values_indexes(std::vector<nlohmann::json>& json_list_values){
    std::vector<std::size_t> removed_index;
    for (auto &json : json_list_values) {
        for (std::size_t i = 0; i < json.size(); ++i) {
            if (json[i].is_null() && std::find(removed_index.begin(), removed_index.end(), i) == removed_index.end()) {
                removed_index.push_back(i);
                for (auto& inner_json : json_list_values) {
                    inner_json.erase(inner_json.begin() + i); // Erase the element at index i
                }
            }
        }
    }
}

YahooTimeseries get_ticker_ts_data(std::string ticker_str_data){
    nlohmann::json json_object = nlohmann::json::parse(ticker_str_data);
    nlohmann::json json_fields = json_object["chart"]["result"][0];
    std::vector<nlohmann::json> json_list_values = {json_fields["timestamp"], 
                                                    json_fields["indicators"]["quote"][0]["open"], 
                                                    json_fields["indicators"]["quote"][0]["low"], 
                                                    json_fields["indicators"]["quote"][0]["high"], 
                                                    json_fields["indicators"]["quote"][0]["close"], 
                                                    json_fields["indicators"]["adjclose"][0]["adjclose"]};
    remove_null_values_indexes(json_list_values);

    std::string ticker = json_object["chart"]["result"][0]["meta"]["symbol"];
    std::vector<std::time_t> dates = json_list_values[0];
    std::vector<double> opens = json_list_values[1];
    std::vector<double> lows = json_list_values[2];
    std::vector<double> highs = json_list_values[3];
    std::vector<double> closes = json_list_values[4];
    std::vector<double> adjcloses =  json_list_values[5];

    YahooTimeseries yt = {ticker, dates, opens, lows, highs, closes, adjcloses};
    std::map<std::time_t, double> dividend_map;
    if (json_object["chart"]["result"][0].contains("events")){
        const auto& dividends = json_object["chart"]["result"][0]["events"]["dividends"];
        for (auto it = dividends.begin(); it != dividends.end(); ++it) {
            std::time_t date = std::stol(it.key());
            dividend_map[date] = it.value()["amount"];
        }
        return yt = {ticker, dates, opens, lows, highs, closes, adjcloses, dividend_map};
    }
    return yt;
}

std::vector<double> get_exponential_moving_average(std::vector<double> prices, double alpha){
    std::vector<double> ema(prices.size());

    if (!prices.empty()) {
        ema[0] = prices[0];

        for (size_t t = 1; t < prices.size(); ++t) {
            ema[t] = alpha * prices[t] + (1 - alpha) * ema[t - 1];
        }
    }

    return ema;
}

size_t get_date_index(std::time_t date, std::vector<std::time_t> dates){
    for (size_t i = 0; i < dates.size(); ++i) {
        if (dates[i] == date) {
            return i;
        }
    }
    fprintf(stderr, "Date not found\n");
    return std::numeric_limits<size_t>::max(); // Return max value to indicate not found
}

 std::map<std::time_t, double> init_map(const YahooTimeseries& ticker_yt){
    std::vector<std::time_t> dates = ticker_yt.get_dates();
    std::map<std::time_t, double> map;
    for (const auto& date: dates)
        map[date] = 0.0;
    return map;
}

// Function to generate a random vector of std::time_t dates
std::vector<std::time_t> generate_random_dates(size_t count, std::time_t start, std::time_t end) {
    std::vector<std::time_t> dates;
    dates.reserve(count);

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<std::time_t> distribution(start, end);

    for (size_t i = 0; i < count; ++i) {
        dates.emplace_back(distribution(generator));
    }
    std::sort(dates.begin(), dates.end());
    return dates;
}

std::vector<std::time_t> extract_first_dates_of_each_month(const std::vector<std::time_t>& dates){
    std::vector<std::time_t> first_dates;
    first_dates.push_back(dates[0]);
    std::tm* time_info = std::localtime(&dates[0]);
    int current_year = time_info->tm_year;
    int current_month = time_info->tm_mon;
    for (const auto& date : dates) {
        time_info = std::localtime(&date);

        if (time_info->tm_year > current_year || time_info->tm_mon > current_month){
            first_dates.push_back(date);
            current_year = time_info->tm_year;
            current_month = time_info->tm_mon;
        }

    }
    return first_dates;
}

std::vector<std::time_t> extract_last_dates_of_each_month(const std::vector<std::time_t>& dates){
    std::vector<std::time_t> copy_of_dates = dates;
    std::reverse(copy_of_dates.begin(), copy_of_dates.end());
    std::vector<std::time_t> last_dates;
    std::tm* time_info = std::localtime(&copy_of_dates[0]);
    last_dates.push_back(copy_of_dates[0]);
    int current_year = time_info->tm_year;
    int current_month = time_info->tm_mon;
    for (const auto& date : copy_of_dates) {
        time_info = std::localtime(&date);

        if (time_info->tm_year < current_year || time_info->tm_mon < current_month){
            last_dates.push_back(date);
            current_year = time_info->tm_year;
            current_month = time_info->tm_mon;
        }
    }
    return last_dates;
}


std::time_t generate_strategy_config_ym_date(int year, int month, int investment_montly_weeknum, int investment_week_day){
    struct tm timeStruct = {0};
    timeStruct.tm_year = year;
    timeStruct.tm_mon = month;
    timeStruct.tm_mday = 1;
    mktime(&timeStruct);

    int dayOfWeek = timeStruct.tm_wday;
    int daysToAdd = ((investment_week_day - dayOfWeek + 7) % 7) + (7 * investment_montly_weeknum);
    timeStruct.tm_mday += daysToAdd;
    return mktime(&timeStruct);
}

std::vector<std::time_t> extract_strategy_config_recurrent_investment_dates(const std::vector<std::time_t> &dates, int investment_nb_months_frequency, int investment_montly_weeknum, int investment_week_day)
{
    std::vector<std::time_t> invest_dates;
    invest_dates.push_back(dates[0]);
    std::tm* time_info = std::localtime(&dates[0]);
    int current_year = time_info->tm_year;
    int current_month = time_info->tm_mon;
    for (size_t i = 1; i < dates.size(); ++i){
        time_info = std::localtime(&dates[i]);
        int next_invest_month = (current_month + investment_nb_months_frequency) % 12;
        int date_m_week = (time_info->tm_mday / 7) + 1; 
        int delta = investment_nb_months_frequency - (11 - current_month);
        int next_invest_year = current_year;

        if (delta > 0)
            next_invest_year = (current_year + (delta / 12) + (delta % 12));

        if (time_info->tm_year != next_invest_year || time_info->tm_mon != next_invest_month)
            continue;

        std::time_t next_target_investment_date = generate_strategy_config_ym_date(next_invest_year, next_invest_month, investment_montly_weeknum, investment_week_day);
        std::time_t next_investment_date = *std::lower_bound(dates.begin() + i, dates.end(), next_target_investment_date);
        invest_dates.push_back(next_investment_date);
        current_month = next_invest_month;
        current_year = next_invest_year;
    }
    return invest_dates;
}

bool almost_equal(double a, double b, double epsilon) {
    return std::abs(a - b) < epsilon;
}

bool vectors_almost_equal(const std::vector<double>& v1, const std::vector<double>& v2, double epsilon) {
    if (v1.size() != v2.size()) return false;

    for (size_t i = 0; i < v1.size(); ++i) {
        if (!almost_equal(v1[i], v2[i], epsilon)) {
            return false;
        }
    }
    return true;
}

double get_standard_deviation(const std::vector<double>& values){
    double mean =  std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    double variance = 0.0;
    for (double value : values) {
        variance += (value - mean) * (value - mean);
    }
    variance /= values.size();
    return std::sqrt(variance);
}

bool contains_all_tickers_yt_vectors(const std::vector<YahooTimeseries>& all_tickers_yt, const std::vector<YahooTimeseries>& sub_tickers_yt) 
{
    if (sub_tickers_yt.size() == 0)
        return true;
    for (const auto& elem : sub_tickers_yt) {
        if (std::find(all_tickers_yt.begin(), all_tickers_yt.end(), elem) == all_tickers_yt.end()) {
            return false; // Element from sub_tickers_yt not found in all_tickers_yt
        }
    }
    return true;
}

nlohmann::json read_json_file(std::string json_filepath)
{
    std::ifstream file(json_filepath); // Open the file
    if (!file) {
        std::cerr << "Error: Unable to open file " << json_filepath << std::endl;
        exit(0);
    }

    nlohmann::json json;
    file >> json;
    return json;
}

std::vector<std::string> parse_string_list(const std::string& input){
    std::vector<std::string> result;
    if (input.empty() || input[0] != '[' || input[input.size() - 1] != ']')
        return result;
    std::string cleaned = input.substr(1, input.size() - 2);  // Remove '[' and ']'

    std::stringstream ss(cleaned);
    std::string token;

    while (std::getline(ss, token, ',')) {
        // Trim leading and trailing spaces
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);

        result.push_back(token);
    }

    return result;
}

std::map<std::string, double> parse_string_map(const std::string& input){
    std::map<std::string, double> result;

    // Remove surrounding brackets
    std::string cleaned = input.substr(1, input.size() - 2);

    std::stringstream ss(cleaned);
    std::string pair;

    while (std::getline(ss, pair, ',')) {
        // Trim leading and trailing spaces
        pair.erase(0, pair.find_first_not_of(" \t"));
        pair.erase(pair.find_last_not_of(" \t") + 1);

        // Find the separator ':'
        size_t pos = pair.find(':');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value_str = pair.substr(pos + 1);

            // Trim spaces again
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value_str.erase(0, value_str.find_first_not_of(" \t"));
            value_str.erase(value_str.find_last_not_of(" \t") + 1);

            // Convert value to double
            try {
                double value = std::stod(value_str);
                result[key] = value;
            } catch (const std::exception& e) {
                std::cerr << "Error converting '" << value_str << "' to double: " << e.what() << std::endl;
            }
        }
    }

    return result;
}

std::vector<std::string> parse_string_array(const std::string& input) {
    std::vector<std::string> result;

    // Ensure the input is well-formed
    if (input.size() < 2 || input.front() != '[' || input.back() != ']') {
        throw std::invalid_argument("Invalid format: Expected [val1, val2]");
    }

    // Extract the inner content (remove the outer '[' and ']')
    std::string content = input.substr(1, input.size() - 2);
    std::string current_token;
    int bracket_depth = 0;

    for (char ch : content) {
        if (ch == '[') {
            bracket_depth++;
        } else if (ch == ']') {
            bracket_depth--;
        }

        if (ch == ',' && bracket_depth == 0) {
            // Push the current token when we reach a top-level comma
            current_token.erase(0, current_token.find_first_not_of(" \t")); // Trim leading spaces
            current_token.erase(current_token.find_last_not_of(" \t") + 1); // Trim trailing spaces
            if (!current_token.empty()) {
                result.push_back(current_token);
            }
            current_token.clear();
        } else {
            current_token += ch;
        }
    }

    // Add the last token
    current_token.erase(0, current_token.find_first_not_of(" \t"));
    current_token.erase(current_token.find_last_not_of(" \t") + 1);
    if (!current_token.empty()) {
        result.push_back(current_token);
    }

    return result;
}

std::vector<std::string> parse_string_array2(const std::string& input){
    std::vector<std::string> result;

    // Ensure the input is well-formed
    if (input.size() < 2 || input.front() != '[' || input.back() != ']') {
        throw std::invalid_argument("Invalid format: Expected [val1, val2]");
    }

    // Extract the inner content (remove '[' and ']')
    std::string content = input.substr(1, input.size() - 2);

    std::stringstream ss(content);
    std::string token;

    while (std::getline(ss, token, ',')) {
        // Trim leading and trailing spaces
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        result.push_back(token);
    }

    return result;
}

std::set<std::string> flatten_to_set(const std::vector<std::vector<std::string>>& list_of_lists){
    std::set<std::string> result;

    for (const auto& sublist : list_of_lists) {
        for (const auto& item : sublist) {
            result.insert(item); // Insert each item into the set
        }
    }

    return result;
}

std::vector<double> get_closest_value(std::time_t date, const std::map<std::time_t, std::vector<double>>& values_map){
    auto it = values_map.find(date);
    
    if (it != values_map.end()) {
        return it->second;
    }

    auto lower = values_map.lower_bound(date);
    if (lower != values_map.end() && lower == values_map.begin()) 
            return lower->second;

    if (lower != values_map.begin()) {
        auto prev = std::prev(lower);
        return prev->second;
    }

    return {0.0, 0.0};
}