#ifndef INPUT_HANDLER
#define INPUT_HANDLER

#include <string>
#include <vector>
#include <map>
#include "../headers/strategy_config.hpp"

class UserInputHandler {

public:
    UserInputHandler(int argc, char* argv[]);
    std::vector<struct GeneralParameters> get_general_parameters();
    std::vector<struct RecurrentInvestmentParameters> get_rinv_parameters();
    std::vector<struct RiskParameters> get_risk_parameters();
    std::vector<struct TechnicalIndicators> get_technical_indicators();
    const std::vector<std::time_t>& get_all_tickers_dates() const;
    ~UserInputHandler();

private:
    size_t nb_strategies;
    std::vector<YahooTimeseries> tickers_ts_data;
    std::vector<std::time_t> all_tickers_dates;
    std::vector<std::map<std::string, std::string>> general_param_args;
    std::vector<std::map<std::string, std::string>> rinv_param_args;
    std::vector<std::map<std::string, std::string>> risk_param_args;
    std::vector<std::map<std::string, std::string>> techind_param_args;
};

#endif