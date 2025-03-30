#include "gtest/gtest.h"
#include "../headers/yahoo_utils.hpp"
#include "../headers/portfolio_builder.hpp"

TEST(PortfolioBuilder, deposit){
    std::tm tm = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::time_t date = std::mktime(&tm);

    std::tm tm2 = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t date2 = std::mktime(&tm2);
    PortfolioBuilder* ptf = new PortfolioBuilder();

    ASSERT_FLOAT_EQ(0.0, ptf->get_cash_amount(date));
    ptf->deposit(200.12, date);
    ptf->deposit(1.0, date);
    ASSERT_FLOAT_EQ(201.12, ptf->get_cash_amount(date));

    ptf->deposit(10.13, date2);
    ASSERT_FLOAT_EQ(201.12, ptf->get_cash_amount(date));
    ASSERT_FLOAT_EQ(211.25, ptf->get_cash_amount(date2));
}

TEST(PortfolioBuilder, buy){
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    PortfolioBuilder* ptf = new PortfolioBuilder();
    YahooTimeseries* yt1 = new YahooTimeseries("TEST_TICKER", 
                                               random_dates, 
                                               {100, 90, 110, 90, 120, 140, 100, 90, 140, 145}, 
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145});

    YahooTimeseries* yt2 = new YahooTimeseries("TEST_TICKER2", 
                                               random_dates, 
                                               {100, 90, 110, 90, 120, 140, 100, 90, 140, 145}, 
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145});
    
    ASSERT_EQ({}, ptf->get_asset("TEST_TICKER"));

    ptf->deposit(90, random_dates[0]);
    ptf->buy(*yt1, 1.0, random_dates[0]);

    AssetHolding* asset = ptf->get_asset("TEST_NoTICKER");
    ASSERT_EQ(nullptr, asset);

    asset = ptf->get_asset("TEST_TICKER");
    ASSERT_EQ(nullptr, asset);
    
    ptf->deposit(10, random_dates[0]);
    ASSERT_FLOAT_EQ(100, ptf->get_cash_amount(random_dates[0]));
    ptf->buy(*yt1, 1.0, random_dates[0]);
    asset = ptf->get_asset("TEST_TICKER");

    ASSERT_FLOAT_EQ(1.0, asset->historical_cumulative_ticker_shares[random_dates[0]]);
    ASSERT_FLOAT_EQ(100.0, asset->historical_cumulative_ticker_expenses[random_dates[0]]);
    ASSERT_FLOAT_EQ(0.0, ptf->get_cash_amount(random_dates[0]));

    std::map<std::time_t, double> to_test_shares = {{random_dates[0], 1.0}};
    std::map<std::time_t, double> to_test_expenses = {{random_dates[0], 100.0}};
    ASSERT_EQ(to_test_shares, asset->historical_cumulative_ticker_shares);
    ASSERT_EQ(to_test_expenses, asset->historical_cumulative_ticker_expenses);

    ptf->deposit(120, random_dates[0]);
    ptf->buy(*yt1, 1.2, random_dates[0]);
    asset = ptf->get_asset("TEST_TICKER");
    ASSERT_FLOAT_EQ(2.2, asset->historical_cumulative_ticker_shares[random_dates[0]]);
    ASSERT_FLOAT_EQ(220.0, asset->historical_cumulative_ticker_expenses[random_dates[0]]);

    ptf->deposit(120, random_dates[0]);
    ptf->buy(*yt1, 1.2, random_dates[2]);
    asset = ptf->get_asset("TEST_TICKER");
    ASSERT_FLOAT_EQ(2.2, asset->historical_cumulative_ticker_shares[random_dates[0]]);
    ASSERT_FLOAT_EQ(220.0, asset->historical_cumulative_ticker_expenses[random_dates[0]]);
    ASSERT_FLOAT_EQ(3.4, asset->historical_cumulative_ticker_shares[random_dates[2]]);
    ASSERT_FLOAT_EQ(340.0, asset->historical_cumulative_ticker_expenses[random_dates[2]]);
    ASSERT_FLOAT_EQ(0.0, ptf->get_cash_amount(random_dates[2]));

    ptf->deposit(100, random_dates[3]);
    ptf->buy(*yt2, 1.22, random_dates[5]);
    asset = ptf->get_asset("TEST_TICKER2");
    ASSERT_FLOAT_EQ(1.22, asset->historical_cumulative_ticker_shares[random_dates[5]]);
    ASSERT_FLOAT_EQ(97.6, asset->historical_cumulative_ticker_expenses[random_dates[5]]);
    ASSERT_FLOAT_EQ(2.4, ptf->get_cash_amount(random_dates[5]));

    asset = ptf->get_asset("TEST_TICKER");
    ASSERT_FLOAT_EQ(2.2, asset->historical_cumulative_ticker_shares[random_dates[0]]);
    ASSERT_FLOAT_EQ(220.0, asset->historical_cumulative_ticker_expenses[random_dates[0]]);
    ASSERT_FLOAT_EQ(3.4, asset->historical_cumulative_ticker_shares[random_dates[2]]);
    ASSERT_FLOAT_EQ(340.0, asset->historical_cumulative_ticker_expenses[random_dates[2]]);

    delete yt1;
    delete yt2;
    delete ptf;
}

TEST(PortfolioBuilder, sell){
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    PortfolioBuilder* ptf = new PortfolioBuilder();
    YahooTimeseries* yt1 = new YahooTimeseries("TEST_TICKER", 
                                               random_dates, 
                                               {100, 90, 110, 90, 120, 140, 100, 90, 140, 145}, 
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145});
    
    
    ptf->deposit(1000, random_dates[0]);
    ptf->buy(*yt1, 2.21, random_dates[0]);
    ptf->sell(*yt1, 0.05, random_dates[1]);
    ptf->sell(*yt1, 3, random_dates[2]);
    
    AssetHolding* asset = ptf->get_asset("TEST_TICKER");

    ASSERT_EQ(asset->historical_cumulative_ticker_shares.find(random_dates[3]), asset->historical_cumulative_ticker_shares.end());
    ASSERT_EQ(asset->historical_cumulative_ticker_expenses.find(random_dates[3]), asset->historical_cumulative_ticker_expenses.end());
    ASSERT_FLOAT_EQ(784, ptf->get_cash_amount(random_dates[2]));
    
    std::map<std::time_t, double> to_test_shares = {{random_dates[0], 2.21}, {random_dates[1], 2.16}};
    std::map<std::time_t, double> to_test_expenses = {{random_dates[0], 221.0}, {random_dates[1], 216.0}};
    ASSERT_FLOAT_EQ(2.16, asset->historical_cumulative_ticker_shares[random_dates[1]]);
    ASSERT_FLOAT_EQ(216, asset->historical_cumulative_ticker_expenses[random_dates[1]]);
    ASSERT_EQ(to_test_shares, asset->historical_cumulative_ticker_shares);
    ASSERT_EQ(to_test_expenses, asset->historical_cumulative_ticker_expenses);

    ptf->sell(*yt1, 2.16, random_dates[2]);
    ASSERT_FLOAT_EQ(0, asset->historical_cumulative_ticker_shares[random_dates[2]]);
    ASSERT_FLOAT_EQ(0, asset->historical_cumulative_ticker_expenses[random_dates[2]]);
    ASSERT_FLOAT_EQ(1000, ptf->get_cash_amount(random_dates[2]));

    ptf->sell(*yt1, 1., random_dates[6]);
    ASSERT_EQ(asset->historical_cumulative_ticker_shares.find(random_dates[6]), asset->historical_cumulative_ticker_shares.end());
    ASSERT_EQ(asset->historical_cumulative_ticker_expenses.find(random_dates[6]), asset->historical_cumulative_ticker_expenses.end());
    ASSERT_FLOAT_EQ(1000, ptf->get_cash_amount(random_dates[6]));
    
    ptf->buy(*yt1, 1.22, random_dates[5]);
    ptf->sell(*yt1, 1., random_dates[6]);
    ASSERT_FLOAT_EQ(0.22, asset->historical_cumulative_ticker_shares[random_dates[6]]);
    ASSERT_FLOAT_EQ(-4.63, asset->historical_cumulative_ticker_expenses[random_dates[6]]);
    ASSERT_FLOAT_EQ(1004.63, ptf->get_cash_amount(random_dates[6]));

    delete yt1;
    delete ptf;
}

TEST(PortfolioBuilder, get_portfolio_value){
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    PortfolioBuilder* ptf = new PortfolioBuilder();
    YahooTimeseries* yt1 = new YahooTimeseries("TEST_TICKER", 
                                               random_dates, 
                                               {100, 90, 110, 90, 120, 140, 100, 90, 140, 145}, 
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145});

    YahooTimeseries* yt2 = new YahooTimeseries("TEST_TICKER2", 
                                               random_dates, 
                                               {100, 90, 110, 90, 120, 140, 100, 90, 140, 145}, 
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145});

    ptf->deposit(1000, random_dates[0]);
    ptf->buy(*yt1, 2.21, random_dates[0]);
    ptf->sell(*yt1, 0.05, random_dates[1]);
    ptf->sell(*yt1, 3, random_dates[2]);

    ASSERT_FLOAT_EQ(1000.0, ptf->get_portfolio_value(random_dates[0]));
    ASSERT_FLOAT_EQ(1000.0, ptf->get_portfolio_value(random_dates[1]));
    ASSERT_FLOAT_EQ(1000.0, ptf->get_portfolio_value(random_dates[2]));

    ptf->sell(*yt1, 2.16, random_dates[2]);
    ASSERT_FLOAT_EQ(1000.0,  ptf->get_portfolio_value(random_dates[2]));

    ptf->sell(*yt1, 1., random_dates[6]);
    ASSERT_FLOAT_EQ(1000.0, ptf->get_portfolio_value(random_dates[6]));

    ptf->buy(*yt1, 1.22, random_dates[5]);
    ptf->sell(*yt1, 1., random_dates[6]);
    ASSERT_FLOAT_EQ(1027.12, ptf->get_portfolio_value(random_dates[6]));

    ptf->buy(*yt2, 1.1, random_dates[6]);
    ASSERT_FLOAT_EQ(1027.12, ptf->get_portfolio_value(random_dates[6]));
    ASSERT_FLOAT_EQ(1010.99, ptf->get_portfolio_value(random_dates[7]));

    delete yt1;
    delete yt2;
    delete ptf;
}

TEST(PortfolioBuilder, get_portfolio_percentage_allocations){
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    PortfolioBuilder* ptf = new PortfolioBuilder();
    YahooTimeseries* yt1 = new YahooTimeseries("TEST_TICKER", 
                                               random_dates, 
                                               {100, 90, 110, 90, 120, 140, 100, 90, 140, 145}, 
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145});

    YahooTimeseries* yt2 = new YahooTimeseries("TEST_TICKER2", 
                                               random_dates, 
                                               {100, 90, 110, 90, 120, 140, 100, 90, 140, 145}, 
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 150},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145});
    
    std::map<std::string, double> pct_alloc = ptf->get_portfolio_percentage_allocations(random_dates[9]);
    std::map<std::string, double> expected;
    ASSERT_EQ(pct_alloc.empty(), true);
    
    ptf->deposit(1000, random_dates[0]);
    pct_alloc = ptf->get_portfolio_percentage_allocations(random_dates[0]);
    expected = {{"Cash", 1.0}};
    ASSERT_EQ(pct_alloc, expected);

    ptf->buy(*yt1, 2.21, random_dates[0]);
    pct_alloc = ptf->get_portfolio_percentage_allocations(random_dates[0]);
    expected = {{"TEST_TICKER", 0.221}, {"Cash", 0.779}};
    ASSERT_EQ(pct_alloc, expected);

    expected = {{"TEST_TICKER", 0.2694}, {"Cash", 0.7306}};
    pct_alloc = ptf->get_portfolio_percentage_allocations(random_dates[4]);
    ASSERT_EQ(almost_equal(expected["TEST_TICKER"], pct_alloc["TEST_TICKER"],  0.0001), true);
    ASSERT_EQ(almost_equal(expected["Cash"], pct_alloc["Cash"],  0.0001), true);

    ptf->buy(*yt2, 1.0, random_dates[2]);
    expected = {{"TEST_TICKER", 0.221}, {"TEST_TICKER2", 0.0}, {"Cash", 0.779}};
    pct_alloc = ptf->get_portfolio_percentage_allocations(random_dates[0]);
    ASSERT_EQ(pct_alloc, expected);

    expected = {{"TEST_TICKER", 0.221}, {"TEST_TICKER2", 0.10}, {"Cash", 0.679}};
    pct_alloc = ptf->get_portfolio_percentage_allocations(random_dates[2]);
    ASSERT_EQ(almost_equal(expected["TEST_TICKER"], pct_alloc["TEST_TICKER"],  0.0001), true);
    ASSERT_EQ(almost_equal(expected["TEST_TICKER2"], pct_alloc["TEST_TICKER2"],  0.0001), true);
    ASSERT_EQ(almost_equal(expected["Cash"], pct_alloc["Cash"],  0.0001), true);

    expected = {{"TEST_TICKER", 0.2788}, {"TEST_TICKER2", 0.1305}, {"Cash", 0.5907}};
    pct_alloc = ptf->get_portfolio_percentage_allocations(random_dates[9]);
    ASSERT_EQ(almost_equal(expected["TEST_TICKER"], pct_alloc["TEST_TICKER"],  0.0001), true);
    ASSERT_EQ(almost_equal(expected["TEST_TICKER2"], pct_alloc["TEST_TICKER2"],  0.0001), true);
    ASSERT_EQ(almost_equal(expected["Cash"], pct_alloc["Cash"],  0.0001), true);
    
    delete yt1;
    delete yt2;
    delete ptf;
}

TEST(PortfolioBuilder, get_portfolio_values){
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    PortfolioBuilder* ptf = new PortfolioBuilder();
    YahooTimeseries* yt1 = new YahooTimeseries("TEST_TICKER", 
                                               random_dates, 
                                               {100, 90, 110, 90, 120, 140, 100, 90, 140, 145}, 
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145});

    YahooTimeseries* yt2 = new YahooTimeseries("TEST_TICKER2", 
                                               random_dates, 
                                               {100, 90, 110, 90, 120, 140, 100, 90, 140, 145}, 
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145});

    ptf->deposit(1000, random_dates[0]);
    ptf->buy(*yt1, 2.21, random_dates[0]);

    Timeseries expected = Timeseries(random_dates, {1000.0, 1000.0, 1000.0, 977.9, 1066.3, 955.8, 1004.93, 977.92, 1088.07, 1099.45});
    Timeseries ptf_global_values = ptf->get_ts_portfolio_values();
    ASSERT_EQ(ptf_global_values, expected);

    ptf->buy(*yt2, 1.0, random_dates[2]);

    expected = Timeseries(random_dates, {1000.0, 1000.0, 1000.0, 967.9, 1096.3, 935.8, 1007.16, 967.93, 1127.92, 1144.45});
    ptf_global_values = ptf->get_ts_portfolio_values();
    ASSERT_EQ(ptf_global_values, expected);

    delete yt1;
    delete yt2;
    delete ptf;
}

TEST(PortfolioBuilder, get_ticker_values){
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 10;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    PortfolioBuilder* ptf = new PortfolioBuilder();
    YahooTimeseries* yt1 = new YahooTimeseries("TEST_TICKER", 
                                               random_dates, 
                                               {100, 90, 110, 90, 120, 140, 100, 90, 140, 145}, 
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145},
                                               {100, 100, 100, 90, 130, 80, 102.23, 90.01, 139.85, 145});

    ptf->deposit(1000, random_dates[0]);
    ptf->buy(*yt1, 2.21, random_dates[0]);

    Timeseries expected = Timeseries(random_dates, {221.0, 221.0, 221.0, 198.9, 287.3, 176.8, 225.93, 198.92, 309.07, 320.45});
    Timeseries ticker_values = ptf->get_ticker_values("TEST_TICKER");
    ASSERT_EQ(ticker_values, expected);

    ptf->buy(*yt1, 1.0, random_dates[2]);
    expected = Timeseries(random_dates, {1000.0, 1000.0, 1000.0, 967.9, 1096.3, 935.8, 1007.16, 967.93, 1127.92, 1144.45});
    ticker_values = ptf->get_ts_portfolio_values();
    ASSERT_EQ(ticker_values, expected);

    ptf->sell(*yt1, 1.0, random_dates[5]);
    expected = Timeseries(random_dates, {1000.0, 1000.0, 1000.0, 967.9, 1096.3, 935.8, 984.93, 957.92, 1068.07, 1079.45});
    ticker_values = ptf->get_ts_portfolio_values();
    ASSERT_EQ(ticker_values, expected);

    ptf->buy(*yt1, 1.0, random_dates[6]);
    ptf->sell(*yt1, 1.0, random_dates[9]);
    expected = Timeseries(random_dates, {1000.0, 1000.0, 1000.0, 967.9, 1096.3, 935.8, 984.93, 945.70, 1105.69, 1122.22});
    ticker_values = ptf->get_ts_portfolio_values();
    ASSERT_EQ(ticker_values, expected);

    delete yt1;
    delete ptf;
}

TEST(PortfolioBuilder, get_ticker_profits_and_losses){
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 7;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    PortfolioBuilder* ptf = new PortfolioBuilder();
    YahooTimeseries* yt1 = new YahooTimeseries("TEST_TICKER", 
                                               random_dates, 
                                               {100, 90, 110, 90, 120, 140, 100}, 
                                               {1, 2, 3, 4, 5, 6, 7},
                                               {1, 2, 3, 4, 5, 6, 7},
                                               {10, 8, 12, 7, 15, 10, 13},
                                               {100, 100, 100, 90, 130, 80, 102.23});
    
    ptf->deposit(1000, random_dates[0]);
    ptf->buy(*yt1, 1, random_dates[0]);
    
    Timeseries ticker_pl_values = ptf->get_ticker_profits_and_losses("TEST_TICKER");
    Timeseries expected = Timeseries(random_dates, {0, -2, 2, -3, 5, 0, 3});
    ASSERT_EQ(ticker_pl_values, expected);

    ptf->buy(*yt1, 1, random_dates[3]);
    ptf->sell(*yt1, 0.5, random_dates[4]);
    ticker_pl_values = ptf->get_ticker_profits_and_losses("TEST_TICKER");
    expected = Timeseries(random_dates, {0, -2, 2, -3, 13, 5.5, 10});
    ASSERT_EQ(ticker_pl_values, expected);

    delete yt1;
    delete ptf;
}

TEST(PortfolioBuilder, get_portfolio_profits_and_losses){
    std::tm tm_start = {0, 0, 0, 1, 0, 120}; // Jan 1, 2020
    std::tm tm_end = {0, 0, 0, 1, 0, 124};   // Jan 1, 2024
    std::time_t start = std::mktime(&tm_start);
    std::time_t end = std::mktime(&tm_end);
    size_t count = 7;
    std::vector<std::time_t> random_dates = generate_random_dates(count, start, end);

    PortfolioBuilder* ptf = new PortfolioBuilder();
    YahooTimeseries* yt1 = new YahooTimeseries("TEST_TICKER", 
                                               random_dates, 
                                               {100, 90, 110, 90, 120, 140, 100}, 
                                               {1, 2, 3, 4, 5, 6, 7},
                                               {1, 2, 3, 4, 5, 6, 7},
                                               {10, 8, 12, 7, 15, 10, 13},
                                               {100, 100, 100, 90, 130, 80, 102.23});

    YahooTimeseries* yt2 = new YahooTimeseries("TEST_TICKER2", 
                                               random_dates, 
                                               {100, 90, 110, 90, 120, 140, 100}, 
                                               {1, 2, 3, 4, 5, 6, 7},
                                               {1, 2, 3, 4, 5, 6, 7},
                                               {10, 8, 12, 7, 15, 10, 13},
                                               {100, 100, 100, 90, 130, 80, 102.23});

    ptf->deposit(1000, random_dates[0]);
    ptf->buy(*yt1, 1, random_dates[0]);
    
    Timeseries ptf_pl_values = ptf->get_portfolio_profits_and_losses();
    Timeseries expected = Timeseries(random_dates, {0, -2, 2, -3, 5, 0, 3});
    ASSERT_EQ(ptf_pl_values, expected);

    ptf->buy(*yt1, 1, random_dates[3]);
    ptf->sell(*yt1, 0.5, random_dates[4]);
    ptf_pl_values = ptf->get_portfolio_profits_and_losses();
    expected = Timeseries(random_dates, {0, -2, 2, -3, 13, 5.5, 10});
    ASSERT_EQ(ptf_pl_values, expected);

    ptf->buy(*yt2, 1, random_dates[4]);
    ptf_pl_values = ptf->get_portfolio_profits_and_losses();
    expected = Timeseries(random_dates, {0, -2, 2, -3, 13, 0.5, 8});
    ASSERT_EQ(ptf_pl_values, expected);

    ptf->buy(*yt2, 1, random_dates[6]);
    ptf_pl_values = ptf->get_portfolio_profits_and_losses();
    expected = Timeseries(random_dates, {0, -2, 2, -3, 13, 0.5, 8});
    ASSERT_EQ(ptf_pl_values, expected);

    delete yt1;
    delete yt2;
    delete ptf;
}