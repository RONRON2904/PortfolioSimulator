#include <nlohmann/json.hpp>
#include <iostream> 
#include <fmt/core.h>
#include "../headers/input_handler.hpp"
#include "../headers/yahoo_utils.hpp"

#define ASSERT_WITH_MSG(cond, msg) do \
{ if (!(cond)) { std::ostringstream str; str << msg; std::cerr << str.str(); std::abort(); } \
} while(0)

UserInputHandler::UserInputHandler(nlohmann::json inputs)
{
    nlohmann::json allowed_params = read_json_file("../allowed_params.json");
    std::set<std::string> general_params_allowed_list = allowed_params["GeneralParameters"];
    std::set<std::string> rinv_params_allowed_list = allowed_params["RecurrentInvestmentParameters"];
    std::set<std::string> risk_params_allowed_list = allowed_params["RiskParameters"];
    std::set<std::string> techind_params_allowed_list = allowed_params["TechnicalIndicators"];
    this->nb_strategies = inputs["nb_strat"];
    std::vector<std::vector<std::string>> all_tickers_vector;
    std::time_t min_start_date, max_end_date;
    
    this->general_param_args.resize(this->nb_strategies);
    this->rinv_param_args.resize(this->nb_strategies);
    this->risk_param_args.resize(this->nb_strategies);
    this->techind_param_args.resize(this->nb_strategies);
    for (auto& [key, value] : inputs.items()){
        if (key == "nb_strat") continue;
        
        std::vector<std::string> values = parse_string_json_array(value.dump());
        bool correct_param = false;
        for (int strat_j = 0; strat_j < this->nb_strategies; ++strat_j){
            std::string val = values[strat_j];
           
            if (std::find(allowed_params["GeneralParameters"].begin(), allowed_params["GeneralParameters"].end(), key) != allowed_params["GeneralParameters"].end()) {
                this->general_param_args[strat_j][key] = val;
                correct_param = true;

                if (key == "all_tickers") {
                    all_tickers_vector.push_back(parse_string_array(val));
                }
                if (key == "start_date") {
                    std::time_t start_date = date_string_to_unix_timestamp(val);
                    if (strat_j == 0)
                        min_start_date = start_date;
                    else
                        min_start_date = std::min(min_start_date, start_date);
                }
                if (key == "end_date") {
                    std::time_t end_date = date_string_to_unix_timestamp(val);
                    if (strat_j == 0)
                        max_end_date = end_date;
                    else
                        max_end_date = std::max(max_end_date, end_date);
                }
            }
            else if (std::find(allowed_params["RecurrentInvestmentParameters"].begin(), allowed_params["RecurrentInvestmentParameters"].end(), key) != allowed_params["RecurrentInvestmentParameters"].end()) {
                this->rinv_param_args[strat_j][key] = val;
                correct_param = true;
            }
            else if (std::find(allowed_params["RiskParameters"].begin(), allowed_params["RiskParameters"].end(), key) != allowed_params["RiskParameters"].end()) {
                this->risk_param_args[strat_j][key] = val;
                correct_param = true;
            }
            else if (std::find(allowed_params["TechnicalIndicators"].begin(), allowed_params["TechnicalIndicators"].end(), key) != allowed_params["TechnicalIndicators"].end()) {
                this->techind_param_args[strat_j][key] = val;
                correct_param = true;
            }
        }
        std::string error_message = fmt::format("Parameter {} is not recognized", value.dump()).c_str();
        ASSERT_WITH_MSG(correct_param, error_message);
    }
    std::set<std::string> flattened_set = flatten_to_set(all_tickers_vector);
    std::vector<std::string> all_tickers_list(flattened_set.begin(), flattened_set.end());
    assert(all_tickers_list.size() >= 1 && "At least one ticker is required. \n");
    YahooFinance *yf = new YahooFinance(all_tickers_list, unix_timestamp_to_date_string(min_start_date), unix_timestamp_to_date_string(max_end_date), "1d");
    this->tickers_ts_data = yf->get_tickers_ts_data();
    this->all_tickers_dates = get_unique_dates(this->tickers_ts_data);
    delete yf;
}

UserInputHandler::UserInputHandler(int argc, char* argv[])
{
    nlohmann::json allowed_params = read_json_file("../allowed_params.json");
    std::set<std::string> general_params_allowed_list = allowed_params["GeneralParameters"];
    std::set<std::string> rinv_params_allowed_list = allowed_params["RecurrentInvestmentParameters"];
    std::set<std::string> risk_params_allowed_list = allowed_params["RiskParameters"];
    std::set<std::string> techind_params_allowed_list = allowed_params["TechnicalIndicators"];
    std::string nb_strat = argv[1];
    size_t eq_pos = nb_strat.find('=');
    this->nb_strategies = std::stoull(nb_strat.substr(eq_pos + 1));
    std::vector<std::vector<std::string>> all_tickers_vector;

    this->general_param_args.resize(this->nb_strategies);
    this->rinv_param_args.resize(this->nb_strategies);
    this->risk_param_args.resize(this->nb_strategies);
    this->techind_param_args.resize(this->nb_strategies);

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        size_t eq_pos = arg.find('=');
        std::string key = arg.substr(2, eq_pos - 2);
        if (arg.rfind("--", 0) == 0){
            bool correct_param = false;
            if (eq_pos != std::string::npos){
                std::vector<std::string> values = parse_string_array(arg.substr(eq_pos + 1));
                for (int strat_j = 0; strat_j < this->nb_strategies; ++strat_j){
                    std::map<std::string, std::string> param;
                    param[key] = values[strat_j];
                    if (std::find(allowed_params["GeneralParameters"].begin(), allowed_params["GeneralParameters"].end(), key) != allowed_params["GeneralParameters"].end())
                    {
                        this->general_param_args[strat_j][key] = values[strat_j];
                        correct_param = true;
                        if (key == "all_tickers")
                            all_tickers_vector.push_back(parse_string_array(values[strat_j]));
                    }
                    else if (std::find(allowed_params["RecurrentInvestmentParameters"].begin(), allowed_params["RecurrentInvestmentParameters"].end(), key) != allowed_params["RecurrentInvestmentParameters"].end())
                    {
                        this->rinv_param_args[strat_j][key] = values[strat_j];
                        correct_param = true;
                    }
                    else if (std::find(allowed_params["RiskParameters"].begin(), allowed_params["RiskParameters"].end(), key) != allowed_params["RiskParameters"].end())
                    {
                        this->risk_param_args[strat_j][key] = values[strat_j];
                        correct_param = true;
                    }
                    else if (std::find(allowed_params["TechnicalIndicators"].begin(), allowed_params["TechnicalIndicators"].end(), key) != allowed_params["TechnicalIndicators"].end())
                    {
                        this->techind_param_args[strat_j][key] = values[strat_j];
                        correct_param = true;
                    }
                }
            }
            std::string error_message = fmt::format("Parameter {} is not recognized", argv[i]).c_str();
            ASSERT_WITH_MSG(correct_param, error_message);
        }
    }
    
    std::set<std::string> flattened_set = flatten_to_set(all_tickers_vector);
    std::vector<std::string> all_tickers_list(flattened_set.begin(), flattened_set.end());
    assert(all_tickers_list.size() >= 1 && "At least one ticker is required. \n");
    YahooFinance *yf = new YahooFinance(all_tickers_list, this->general_param_args[0]["start_date"], this->general_param_args[0]["end_date"], "1d");
    this->tickers_ts_data = yf->get_tickers_ts_data();
    this->all_tickers_dates = get_unique_dates(this->tickers_ts_data);
    delete yf;
}

std::vector<struct GeneralParameters> UserInputHandler::get_general_parameters()
{
    std::vector<struct GeneralParameters> general_params;
    for (int strat_j = 0; strat_j < this->nb_strategies; ++strat_j)
    {   
        std::vector<YahooTimeseries> strat_tickers_yt;
        std::vector<std::string> all_tickers_list = parse_string_list(this->general_param_args[strat_j]["all_tickers"]);
        for (const auto& ticker_yt: this->tickers_ts_data){
            if (std::find(all_tickers_list.begin(), all_tickers_list.end(), ticker_yt.get_ticker()) != all_tickers_list.end())
                strat_tickers_yt.push_back(ticker_yt);
        }
        GeneralParameters strat_general_params(strat_tickers_yt,
                                               date_string_to_unix_timestamp(this->general_param_args[strat_j]["start_date"]),
                                               date_string_to_unix_timestamp(this->general_param_args[strat_j]["end_date"]),
                                               this->general_param_args[strat_j]["strategy_name"], 
                                               std::stod(this->general_param_args[strat_j]["starting_amount"]), 
                                               std::stod(this->general_param_args[strat_j]["monthly_deposit_amount"]), 
                                               std::stod(this->general_param_args[strat_j]["fees_per_trade"]), 
                                               std::stod(this->general_param_args[strat_j]["flat_tax"]), 
                                               this->general_param_args[strat_j]["reinvestment_policy"] == "true");
        general_params.push_back(strat_general_params);
        
    }
    return general_params;
}

std::vector<struct RecurrentInvestmentParameters> UserInputHandler::get_rinv_parameters()
{
    std::vector<struct RecurrentInvestmentParameters> rinv_params;
    for (int strat_j = 0; strat_j < this->nb_strategies; ++strat_j)
    {
        std::vector<std::string> rinv_tickers_list = parse_string_list(this->rinv_param_args[strat_j]["rinv_tickers"]);
        if (rinv_tickers_list.empty())
            rinv_params.push_back(RecurrentInvestmentParameters());
        std::vector<YahooTimeseries> rinv_tickers_ts_data;
        for (const auto& ticker: rinv_tickers_list)
        {
            for (const auto& ticker_yt : this->tickers_ts_data)
            {
                if (ticker_yt.get_ticker() == ticker)
                    rinv_tickers_ts_data.push_back(ticker_yt);
            }
        }
        
        size_t investment_nb_months_frequency, investment_montly_weeknum, investment_week_day, rebalancing_freq, withdraw_nb_months_frequency, withdraw_montly_weeknum, withdraw_week_day;
        std::string rinv_investment_nb_months_frequency = this->rinv_param_args[strat_j]["rinv_investment_nb_months_frequency"];
        std::string rinv_investment_montly_weeknum = this->rinv_param_args[strat_j]["rinv_investment_montly_weeknum"];
        std::string rinv_investment_week_day = this->rinv_param_args[strat_j]["rinv_investment_week_day"];
        std::string rinv_rebalancing_freq_nb_day = this->rinv_param_args[strat_j]["rinv_rebalancing_freq_nb_day"];
        std::string rinv_withdraw_nb_months_frequency = this->rinv_param_args[strat_j]["rinv_withdrawal_nb_months_frequency"];
        std::string rinv_withdraw_montly_weeknum = this->rinv_param_args[strat_j]["rinv_withdrawal_monthly_weeknum"];
        std::string rinv_withdraw_week_day = this->rinv_param_args[strat_j]["rinv_withdrawal_week_day"];
        
        std::stringstream stream(rinv_investment_nb_months_frequency);
        stream >> investment_nb_months_frequency;

        std::stringstream stream2(rinv_investment_montly_weeknum);
        stream2 >> investment_montly_weeknum;

        std::stringstream stream3(rinv_investment_week_day);
        stream3 >> investment_week_day;

        std::stringstream stream4(rinv_rebalancing_freq_nb_day);
        stream4 >> rebalancing_freq;

        std::stringstream stream5(rinv_withdraw_nb_months_frequency);
        stream5 >> withdraw_nb_months_frequency;

        std::stringstream stream6(rinv_withdraw_montly_weeknum);
        stream6 >> withdraw_montly_weeknum;

        std::stringstream stream7(rinv_withdraw_week_day);
        stream7 >> withdraw_week_day;

        RecurrentInvestmentParameters strat_rinv_params(rinv_tickers_ts_data, 
                                                        std::stod(this->rinv_param_args[strat_j]["rinv_starting_amount"]), 
                                                        std::stod(this->rinv_param_args[strat_j]["rinv_investment_amount"]), 
                                                        investment_nb_months_frequency, 
                                                        investment_montly_weeknum, 
                                                        investment_week_day, 
                                                        std::stod(this->rinv_param_args[strat_j]["rinv_rebalancing_threshold"]), 
                                                        rebalancing_freq, 
                                                        parse_string_map(this->rinv_param_args[strat_j]["rinv_assets_desired_pct_allocations"]),
                                                        std::stod(this->rinv_param_args[strat_j]["rinv_withdrawal_pct"]),
                                                        std::stod(this->rinv_param_args[strat_j]["rinv_withdrawal_amount"]),
                                                        withdraw_nb_months_frequency,
                                                        withdraw_montly_weeknum,
                                                        withdraw_week_day);
        rinv_params.push_back(strat_rinv_params);
    }
    return rinv_params;
}

std::vector<struct RiskParameters> UserInputHandler::get_risk_parameters()
{
    std::vector<struct RiskParameters> risk_params;
    for (int strat_j = 0; strat_j < this->nb_strategies; ++strat_j)
    {
        std::vector<std::string> risk_tickers_list = parse_string_list(this->risk_param_args[strat_j]["risk_tickers"]);
        if (risk_tickers_list.empty()){
            risk_params.push_back(RiskParameters());
            continue;
        }
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
        std::string risk_pct_change_window = this->risk_param_args[strat_j]["risk_pct_changes_window"];
        std::stringstream stream(risk_pct_change_window);
        stream >> pct_changes_window;

        RiskParameters strat_risk_params(risk_tickers_ts_data, 
                                        pct_changes_window, 
                                        std::stod(this->risk_param_args[strat_j]["risk_stop_loss_percentage"]),
                                        std::stod(this->risk_param_args[strat_j]["risk_take_profit_percentage"]));
        risk_params.push_back(strat_risk_params);
    }
    return risk_params;
}

std::vector<struct TechnicalIndicators> UserInputHandler::get_technical_indicators()
{
    std::vector<struct TechnicalIndicators> techind_params;
    for (int strat_j = 0; strat_j < this->nb_strategies; ++strat_j)
    {
        std::vector<std::string> techind_rsi_tickers_list = parse_string_list(this->techind_param_args[strat_j]["techind_rsi_tickers"]);
        std::vector<std::string> techind_sma_tickers_list = parse_string_list(this->techind_param_args[strat_j]["techind_sma_tickers"]);
        std::vector<std::string> techind_rsi_sma_tickers_list = parse_string_list(this->techind_param_args[strat_j]["techind_rsi_sma_tickers"]);

        size_t rsi_period, sma_period;
        std::string techind_rsi_period = this->techind_param_args[strat_j]["techind_rsi_period"];
        std::string techind_sma_period = this->techind_param_args[strat_j]["techind_sma_period"];

        std::stringstream stream(techind_rsi_period);
        stream >> rsi_period;

        std::stringstream stream2(techind_sma_period);
        stream2 >> sma_period;

        if (rsi_period == 0 && sma_period == 0){
            techind_params.push_back(TechnicalIndicators());
            continue;
        }
        std::vector<YahooTimeseries> techind_rsi_tickers_ts_data;
        std::vector<YahooTimeseries> techind_sma_tickers_ts_data;
        std::vector<YahooTimeseries> techind_rsi_sma_tickers_ts_data;
        if (techind_rsi_tickers_list.size() > 0 && rsi_period > 0)
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

        if (techind_sma_tickers_list.size() > 0 && sma_period > 0)
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

        if (techind_rsi_sma_tickers_list.size() > 0 && rsi_period > 0 && sma_period > 0)
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

        TechnicalIndicators strat_techind_params(techind_rsi_tickers_ts_data,
                                                techind_sma_tickers_ts_data,
                                                techind_rsi_sma_tickers_ts_data, 
                                                rsi_period,
                                                sma_period,
                                                std::stod(this->techind_param_args[strat_j]["techind_rsi_buy_threshold"]),
                                                std::stod(this->techind_param_args[strat_j]["techind_rsi_sell_threshold"]));
        techind_params.push_back(strat_techind_params);
    }
    return techind_params;
}

const std::vector<std::time_t>& UserInputHandler::get_all_tickers_dates() const{
    return this->all_tickers_dates;
}

UserInputHandler::~UserInputHandler(){}