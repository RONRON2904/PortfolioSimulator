#ifndef INPUT_HANDLER
#define INPUT_HANDLER

#include <string>
#include <vector>
#include <map>
#include "../headers/strategy_config.hpp"

class UserInputHandler {

public:
    UserInputHandler(int argc, char* argv[]);
    UserInputHandler(nlohmann::json args);
    struct GeneralParameters get_general_parameters();
    struct RecurrentInvestmentParameters get_rinv_parameters();
    struct RiskParameters get_risk_parameters();
    struct TechnicalIndicators get_technical_indicators();
    ~UserInputHandler();

private:
    std::vector<YahooTimeseries> tickers_ts_data;
    std::map<std::string, std::string> general_param_args;
    std::map<std::string, std::string> rinv_param_args;
    std::map<std::string, std::string> risk_param_args;
    std::map<std::string, std::string> techind_param_args;
};

#endif