#include "../headers/yahoo_utils.hpp"
#include <curl/curl.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <nlohmann/json.hpp>
#include <vector>
#include <random>

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

    std::cout << url << std::endl;

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

YahooTimeseries get_ticker_ts_data(std::string ticker_str_data){
    nlohmann::json json_object = nlohmann::json::parse(ticker_str_data);

    // Accessing values
    std::string ticker = json_object["chart"]["result"][0]["meta"]["symbol"];
    std::vector<std::time_t> dates = json_object["chart"]["result"][0]["timestamp"];
    std::vector<double> opens = json_object["chart"]["result"][0]["indicators"]["quote"][0]["open"];
    std::vector<double> lows = json_object["chart"]["result"][0]["indicators"]["quote"][0]["low"];
    std::vector<double> highs = json_object["chart"]["result"][0]["indicators"]["quote"][0]["high"];
    std::vector<double> closes = json_object["chart"]["result"][0]["indicators"]["quote"][0]["close"];
    std::vector<double> adjcloses = json_object["chart"]["result"][0]["indicators"]["adjclose"][0]["adjclose"];

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