#include <nlohmann/json.hpp>
#include <iostream> 
#include <fmt/core.h>
#include "../headers/input_handler.hpp"
#include "../headers/yahoo_utils.hpp"

#define ASSERT_WITH_MSG(cond, msg) do \
{ if (!(cond)) { std::ostringstream str; str << msg; std::cerr << str.str(); std::abort(); } \
} while(0)

UserInputHandler::UserInputHandler(int argc, char* argv[])
{
    nlohmann::json allowed_params = read_json_file("../allowed_params.json");
    std::set<std::string> general_params_allowed_list = allowed_params["GeneralParameters"];
    std::set<std::string> rinv_params_allowed_list = allowed_params["RecurrentInvestmentParameters"];
    std::set<std::string> risk_params_allowed_list = allowed_params["RiskParameters"];
    std::set<std::string> techind_params_allowed_list = allowed_params["TechnicalIndicators"];
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        bool correct_param = false;
        if (arg.rfind("--", 0) == 0)
        {
            size_t eq_pos = arg.find('=');
            if (eq_pos != std::string::npos)
            {
                std::string key = arg.substr(2, eq_pos - 2);
                std::string value = arg.substr(eq_pos + 1);
                //if (general_params_allowed_list.find(key) == general_params_allowed_list.end())
                if (std::find(allowed_params["GeneralParameters"].begin(), allowed_params["GeneralParameters"].end(), key) != allowed_params["GeneralParameters"].end())
                {
                    this->general_param_args[key] = value;
                    correct_param = true;
                }
                //else if (rinv_param_args.find(key) == rinv_param_args.end())
                else if (std::find(allowed_params["RecurrentInvestmentParameters"].begin(), allowed_params["RecurrentInvestmentParameters"].end(), key) != allowed_params["RecurrentInvestmentParameters"].end())
                {
                    this->rinv_param_args[key] = value;
                    correct_param = true;
                }
                //else if (risk_param_args.find(key) == risk_param_args.end())
                else if (std::find(allowed_params["RiskParameters"].begin(), allowed_params["RiskParameters"].end(), key) != allowed_params["RiskParameters"].end())
                {
                    this->risk_param_args[key] = value;
                    correct_param = true;
                }
                //else if (techind_param_args.find(key) == techind_param_args.end())
                else if (std::find(allowed_params["TechnicalIndicators"].begin(), allowed_params["TechnicalIndicators"].end(), key) != allowed_params["TechnicalIndicators"].end())
                {
                    this->techind_param_args[key] = value;
                    correct_param = true;
                }
            }
        }
        std::string error_message = fmt::format("Parameter {} is not recognized", argv[i]).c_str();
        ASSERT_WITH_MSG(correct_param, error_message);
    }
    std::vector<std::string> all_tickers_list = parse_string_list(this->general_param_args["all_tickers"]);
    assert(all_tickers_list.size() > 1 && "At least one ticker is required. \n");
    YahooFinance *yf = new YahooFinance(all_tickers_list, this->general_param_args["start_date"], this->general_param_args["end_date"], "1d");
    this->tickers_ts_data = yf->get_tickers_ts_data();
    delete yf;
}

struct GeneralParameters UserInputHandler::get_general_parameters()
{
    GeneralParameters general_params(this->tickers_ts_data, 
                                     this->general_param_args["strategy_name"], 
                                     std::stod(this->general_param_args["starting_amount"]), 
                                     std::stod(this->general_param_args["monthly_deposit_amount"]), 
                                     std::stod(this->general_param_args["fees_per_trade"]), 
                                     std::stod(this->general_param_args["flat_tax"]), 
                                     this->general_param_args["reinvestment_policy"] == "true");
    return general_params;
}

struct RecurrentInvestmentParameters UserInputHandler::get_rinv_parameters()
{
    std::vector<std::string> rinv_tickers_list = parse_string_list(this->rinv_param_args["rinv_tickers"]);
    if (rinv_tickers_list.empty())
        return RecurrentInvestmentParameters();
    std::vector<YahooTimeseries> rinv_tickers_ts_data;
    for (const auto& ticker: rinv_tickers_list)
    {
        for (const auto& ticker_yt : this->tickers_ts_data)
        {
            if (ticker_yt.get_ticker() == ticker)
                rinv_tickers_ts_data.push_back(ticker_yt);
        }
    }
    
    size_t investment_nb_months_frequency, investment_montly_weeknum, investment_week_day, rebalancing_freq;
    std::string rinv_investment_nb_months_frequency = this->rinv_param_args["rinv_investment_nb_months_frequency"];
    std::string rinv_investment_montly_weeknum = this->rinv_param_args["rinv_investment_montly_weeknum"];
    std::string rinv_investment_week_day = this->rinv_param_args["rinv_investment_week_day"];
    std::string rinv_rebalancing_freq_nb_day = this->rinv_param_args["rinv_rebalancing_freq_nb_day"];
    
    std::stringstream stream(rinv_investment_nb_months_frequency);
    stream >> investment_nb_months_frequency;

    std::stringstream stream2(rinv_investment_montly_weeknum);
    stream2 >> investment_montly_weeknum;

    std::stringstream stream3(rinv_investment_week_day);
    stream3 >> investment_week_day;

    std::stringstream stream4(rinv_rebalancing_freq_nb_day);
    stream4 >> rebalancing_freq;

    RecurrentInvestmentParameters rinv_params(rinv_tickers_ts_data, 
                                              std::stod(this->rinv_param_args["rinv_starting_amount"]), 
                                              std::stod(this->rinv_param_args["rinv_investment_amount"]), 
                                              investment_nb_months_frequency, 
                                              investment_montly_weeknum, 
                                              investment_week_day, 
                                              std::stod(this->rinv_param_args["rinv_rebalancing_threshold"]), 
                                              rebalancing_freq, 
                                              parse_string_map(this->rinv_param_args["rinv_assets_desired_pct_allocations"]));
    return rinv_params;
}

struct RiskParameters UserInputHandler::get_risk_parameters()
{
    std::vector<std::string> risk_tickers_list = parse_string_list(this->risk_param_args["risk_tickers"]);
    if (risk_tickers_list.empty())
        return RiskParameters();
    std::vector<YahooTimeseries> risk_tickers_ts_data;
    for (const auto& ticker: risk_tickers_list)
    {
        for (const auto& ticker_yt : this->tickers_ts_data)
        {
            if (ticker_yt.get_ticker() == ticker)
                risk_tickers_ts_data.push_back(ticker_yt);
        }
    }
    size_t pct_changes_window;
    std::string risk_pct_change_window = this->risk_param_args["risk_pct_changes_window"];
    std::stringstream stream(risk_pct_change_window);
    stream >> pct_changes_window;

    RiskParameters risk_params(risk_tickers_ts_data, 
                               pct_changes_window, 
                               std::stod(this->risk_param_args["risk_stop_loss_percentage"]),
                               std::stod(this->risk_param_args["risk_take_profit_percentage"]));
    return risk_params;
}

struct TechnicalIndicators UserInputHandler::get_technical_indicators()
{
    std::vector<std::string> techind_rsi_tickers_list = parse_string_list(this->risk_param_args["techind_rsi_tickers"]);
    std::vector<std::string> techind_sma_tickers_list = parse_string_list(this->risk_param_args["techind_sma_tickers"]);
    std::vector<std::string> techind_rsi_sma_tickers_list = parse_string_list(this->risk_param_args["techind_rsi_sma_tickers"]);

    if (techind_rsi_tickers_list.empty() && techind_sma_tickers_list.empty() && techind_rsi_sma_tickers_list.empty())
        return TechnicalIndicators();

    std::vector<YahooTimeseries> techind_rsi_tickers_ts_data;
    std::vector<YahooTimeseries> techind_sma_tickers_ts_data;
    std::vector<YahooTimeseries> techind_rsi_sma_tickers_ts_data;
    if (techind_rsi_tickers_list.size() > 0)
    {
        for (const auto& ticker: techind_rsi_tickers_list)
        {
            for (const auto& ticker_yt : this->tickers_ts_data)
            {
                if (ticker_yt.get_ticker() == ticker)
                    techind_rsi_tickers_ts_data.push_back(ticker_yt);
            }
        }
    }

    if (techind_sma_tickers_ts_data.size() > 0)
    {
        for (const auto& ticker: techind_sma_tickers_list)
        {
            for (const auto& ticker_yt : this->tickers_ts_data)
            {
                if (ticker_yt.get_ticker() == ticker)
                    techind_sma_tickers_ts_data.push_back(ticker_yt);
            }
        }
    }

    if (techind_rsi_sma_tickers_ts_data.size() > 0)
    {
        for (const auto& ticker: techind_rsi_sma_tickers_list)
        {
            for (const auto& ticker_yt : this->tickers_ts_data)
            {
                if (ticker_yt.get_ticker() == ticker)
                    techind_rsi_sma_tickers_ts_data.push_back(ticker_yt);
            }
        }
    }

    size_t rsi_period, sma_period;
    std::string techind_rsi_period = this->risk_param_args["techind_rsi_period"];
    std::string techind_sma_period = this->risk_param_args["techind_sma_period"];

    std::stringstream stream(techind_rsi_period);
    stream >> rsi_period;

    std::stringstream stream2(techind_sma_period);
    stream2 >> sma_period;

    TechnicalIndicators techind_params(techind_rsi_tickers_ts_data,
                                       techind_sma_tickers_ts_data,
                                       techind_rsi_sma_tickers_ts_data, 
                                       rsi_period,
                                       sma_period,
                                       std::stod(this->risk_param_args["techind_rsi_buy_threshold"]),
                                       std::stod(this->risk_param_args["techind_rsi_sell_threshold"]),
                                       std::stod(this->risk_param_args["techind_rsi_starting_amount"]),
                                       std::stod(this->risk_param_args["techind_sma_starting_amount"]),
                                       std::stod(this->risk_param_args["techind_rsi_sma_starting_amount"]));
    return techind_params;
}

UserInputHandler::~UserInputHandler(){}