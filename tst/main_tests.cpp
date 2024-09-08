#include "gtest/gtest.h"

//g++ -g *.cpp ../src/yahoo_timeseries.cpp ../src/portfolio_builder.cpp ../src/yahoo_utils.cpp -o main -lgtest -lcurl

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}