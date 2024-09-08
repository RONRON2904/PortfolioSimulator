#include "gtest/gtest.h"
#include "../headers/yahoo_timeseries.hpp"
#include "../headers/yahoo_utils.hpp"

#include <vector>
#include <ctime>
#include <algorithm>
#include <iostream>

TEST(Timeseries, get_simple_moving_averages) {
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    Timeseries* ts =  new Timeseries(random_dates, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    std::vector<double> sams = ts->get_simple_moving_averages(3);
    std::vector<double> expected = {2, 3, 4, 5, 6, 7, 8, 9};
    EXPECT_EQ(expected, sams);
    delete ts;
}

TEST(Timeseries, get_ts_simple_moving_averages) {
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    Timeseries* ts =  new Timeseries(random_dates, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    std::map<std::time_t, double> sams = ts->get_ts_simple_moving_averages(3);
    std::map<std::time_t, double> expected = {{random_dates[3], 2}, {random_dates[4], 3}, {random_dates[5], 4}, {random_dates[6], 5}, {random_dates[7], 6}, {random_dates[8], 7}, {random_dates[9], 8}};
    EXPECT_EQ(expected, sams);
    delete ts;
}

TEST(Timeseries, pget_simple_moving_averages) {
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    Timeseries* ts =  new Timeseries(random_dates, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    std::vector<double> sams = ts->pget_simple_moving_averages(3);
    std::vector<double> expected = {2, 3, 4, 5, 6, 7, 8, 9};
    EXPECT_EQ(expected, sams);
    delete ts;
}

TEST(Timeseries, get_exponential_moving_averages) {
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    Timeseries* ts =  new Timeseries(random_dates, {100, 105, 110, 115, 120, 125, 130, 135, 140, 145});                          
    std::vector<double> emas = ts->get_exponential_moving_averages(5);
    std::vector<double> expected = {110.0, 115.0, 120.0, 125.0, 130.0, 135.0};
    for (size_t i=0; i < emas.size(); ++i){
        ASSERT_NEAR(emas[i], expected[i], 0.01);
    }
    delete ts;
}

TEST(Timeseries, get_ts_exponential_moving_averages) {
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    Timeseries* ts =  new Timeseries(random_dates, {100, 105, 110, 115, 120, 125, 130, 135, 140, 145});                          
    std::map<std::time_t, double> emas = ts->get_ts_exponential_moving_averages(5);
    std::map<std::time_t, double> expected = {{random_dates[5], 110.0}, {random_dates[6], 115.0}, {random_dates[7], 120.0}, {random_dates[8], 125.0}, {random_dates[9], 130.0}};
    for (size_t i=0; i < emas.size(); ++i)
        ASSERT_NEAR(emas[random_dates[i]], expected[random_dates[i]], 0.01);
    delete ts;
}

TEST(Timeseries, get_maximum_drowdowns) {
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    Timeseries* ts =  new Timeseries(random_dates, {100, 90, 110, 90, 120, 140, 100, 90, 140, 145});
                                               
    std::vector<double> mxd2 = ts->get_maximum_drawdowns(2);
    std::vector<double> expected2 = {10, 0, 20, 0, 0, 40, 10, 0, 0};
    EXPECT_EQ(expected2, mxd2);

    std::vector<double> mxd4 = ts->get_maximum_drawdowns(4);
    std::vector<double> expected4 = {20, 20, 20, 40, 50, 50, 10};
    EXPECT_EQ(expected4, mxd4);
    delete ts;
}

TEST(Timeseries, get_ts_maximum_drowdowns) {
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    Timeseries* ts =  new Timeseries(random_dates, {100, 90, 110, 90, 120, 140, 100, 90, 140, 145});
                                               
    std::map<std::time_t, double> mxd2 = ts->get_ts_maximum_drawdowns(2);
    std::map<std::time_t, double> expected2 = {{random_dates[2], 10}, {random_dates[3], 0}, {random_dates[4], 20}, {random_dates[5], 0}, {random_dates[6], 0}, {random_dates[7], 40}, {random_dates[8], 10}, {random_dates[9], 0}};
    EXPECT_EQ(expected2, mxd2);

    std::map<std::time_t, double> mxd4 = ts->get_ts_maximum_drawdowns(4);
    std::map<std::time_t, double> expected4 = {{random_dates[4], 20}, {random_dates[5], 20}, {random_dates[6], 20}, {random_dates[7], 40}, {random_dates[8], 50}, {random_dates[9], 50}};
    EXPECT_EQ(expected4, mxd4);
    delete ts;
}

TEST(Timeseries, get_pct_changes) {
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    Timeseries* ts =  new Timeseries(random_dates, {100, 90, 110, 90, 120, 140, 100, 90, 140, 145});

    std::vector<double> pct_changes = ts->get_pct_changes();
    std::vector<double> expected = {-0.1, 20.0/90.0, -20.0/110.0, 30.0/90.0, 20.0/120.0, -40.0/140.0, -0.1, 50.0/90.0, 5.0/140.0};
    EXPECT_EQ(expected, pct_changes);
    delete ts;
}

TEST(Timeseries, get_ts_pct_changes) {
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    Timeseries* ts =  new Timeseries(random_dates, {100, 90, 110, 90, 120, 140, 100, 90, 140, 145});

    std::map<std::time_t, double> pct_changes = ts->get_ts_pct_changes();
    std::map<std::time_t, double> expected = {{random_dates[2], -0.1}, {random_dates[3], 20.0/90.0}, {random_dates[4], -20.0/110.0}, {random_dates[5], 30.0/90.0}, {random_dates[6], 20.0/120.0}, {random_dates[7], -40.0/140.0}, {random_dates[8], -0.1}, {random_dates[9], 50.0/90.0}};
    EXPECT_EQ(expected, pct_changes);
    delete ts;
}

TEST(Timeseries, get_volatilities) {
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    Timeseries* ts =  new Timeseries(random_dates, {100, 90, 110, 90, 120, 140, 100, 90, 140, 145});

    std::vector<double> vols = ts->get_volatilities(4);
    std::vector<double> expected = {8.29, 12.99, 18.03, 19.20, 19.20, 22.78, 24.08};
    for (size_t i=0; i < vols.size(); ++i){
        ASSERT_NEAR(vols[i], expected[i], 0.01);
    }
    delete ts;
}

TEST(Timeseries, get_ts_volatilities) {
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    Timeseries* ts =  new Timeseries(random_dates, {100, 90, 110, 90, 120, 140, 100, 90, 140, 145});

    std::map<std::time_t, double> vols = ts->get_ts_volatilities(4);
    std::map<std::time_t, double> expected = {{random_dates[4], 8.29}, {random_dates[5], 12.99}, {random_dates[6], 18.03}, {random_dates[7], 19.20}, {random_dates[8], 19.20}, {random_dates[9], 22.78}};
    for (size_t i=0; i < vols.size(); ++i){
        ASSERT_NEAR(vols[random_dates[i+4]], expected[random_dates[i+4]], 0.01);
    }
    delete ts;
}

TEST(Timeseries, get_rsis) {
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 22;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    Timeseries* ts =  new Timeseries(random_dates, {283.46, 280.69, 285.48, 294.08, 293.90, 299.92, 301.15, 284.45, 294.09, 302.77, 301.97, 
                                                    306.85, 305.02, 301.06, 291.97, 284.18, 286.48, 284.54, 276.82, 284.49, 275.01, 279.07});

    std::vector<double> rsis = ts->get_rsis(14);
    std::vector<double> expected = {55.37, 50.07, 51.55, 50.20, 45.14, 50.48, 44.69, 47.47};
    for (size_t i=0; i < rsis.size(); ++i)
        ASSERT_NEAR(expected[i], rsis[i], 0.01);

    delete ts;
}

TEST(Timeseries, get_ts_rsis) {
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 22;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    Timeseries* ts =  new Timeseries(random_dates, {283.46, 280.69, 285.48, 294.08, 293.90, 299.92, 301.15, 284.45, 294.09, 302.77, 301.97, 
                                                    306.85, 305.02, 301.06, 291.97, 284.18, 286.48, 284.54, 276.82, 284.49, 275.01, 279.07});

    std::map<std::time_t, double> rsis = ts->get_ts_rsis(14);
    std::map<std::time_t, double> expected = {{random_dates[14], 55.37}, {random_dates[15], 50.07}, {random_dates[16], 51.55}, {random_dates[17], 50.20}, {random_dates[18], 45.14}, {random_dates[19], 50.48}, {random_dates[20], 44.69}, {random_dates[21], 47.47}};
    for (size_t i=0; i < rsis.size(); ++i){
        ASSERT_NEAR(expected[random_dates[i+14]], rsis[random_dates[i+14]], 0.01);
    }
    delete ts;
}

TEST(YahooTimeseries, get_open_close_spreads) {
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    YahooTimeseries* yt =  new YahooTimeseries("TEST_TICKER", 
                                               random_dates, 
                                               {100, 90, 110, 90, 120, 140, 100, 90, 140, 145}, 
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145});

    Timeseries spread = yt->get_open_close_spreads();
    Timeseries expected = Timeseries(random_dates, {0, 10, -10, 0, 10, -60, 2.23, 0.01, -0.15, 0});
    ASSERT_EQ(spread, expected);
    delete yt;
}

TEST(YahooTimeseries, get_high_low_spreads) {
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    YahooTimeseries* yt =  new YahooTimeseries("TEST_TICKER", 
                                               random_dates, 
                                               {100, 90, 110, 90, 120, 140, 100, 90, 140, 145}, 
                                               {1, 2.12, 3.14, 40, 5, 6, 7.10, 8.01, 9, 10},
                                               {1.3, 2, 3, 42, 5.05, 6.32, 7, 8, 9.9, 10},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145});

    Timeseries spread = yt->get_high_low_spreads();
    Timeseries expected = Timeseries(random_dates, {0.3, -0.12, -0.14, 2, 0.05, 0.32, -0.1, -0.01, 0.9, 0});
    ASSERT_EQ(spread, expected);
    delete yt;
}