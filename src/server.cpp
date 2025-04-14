#include <iostream>
#include <httplib.h>
#include <cstdlib>
#include <sstream>  
#include <nlohmann/json.hpp>

#include "../headers/yahoo_utils.hpp"
#include "../headers/input_handler.hpp"
#include "../headers/strategy.hpp"

nlohmann::json merge_strats_jsonOLD(std::vector<std::time_t> &all_dates, 
                                 std::map<std::string, std::map<std::time_t, std::vector<double>>> strats_values, 
                                 std::map<std::string, std::vector<double>> strats_perfs){
    nlohmann::json result = nlohmann::json::array();
    for (const auto& date : all_dates) {
        nlohmann::json entry;
        entry["time"] = unix_timestamp_to_date_string(date);

        for (const auto& [strat_name, values_map] : strats_values) {
            std::vector<double> closest_value = get_closest_value(date, values_map);
            entry[strat_name] = closest_value;
        }

        result.push_back(entry);
    }
    return result;
}

nlohmann::json merge_strats_json(std::vector<std::time_t> &all_dates, 
                                 std::map<std::string, std::map<std::time_t, std::vector<double>>> strats_values,
                                 std::map<std::string, std::vector<double>> strats_perfs) {
    nlohmann::json strat_values_array = nlohmann::json::array();
    nlohmann::json strat_perfs_array = nlohmann::json::array();

    for (const auto& date : all_dates) {
        nlohmann::json strat_entry;
        strat_entry["time"] = unix_timestamp_to_date_string(date);

        for (const auto& [strat_name, values_map] : strats_values) {
            std::vector<double> closest_value = get_closest_value(date, values_map);
            strat_entry[strat_name] = closest_value;
        }

        strat_values_array.push_back(strat_entry);
    }

    for (const auto& [strat_name, perf_values] : strats_perfs) {
        nlohmann::json perf_entry;
        perf_entry["strategy_name"] = strat_name;
        perf_entry["total_returns"] = perf_values[0];
        perf_entry["xirr"] = perf_values[1];
        perf_entry["total_investments"] = perf_values[2];
        strat_perfs_array.push_back(perf_entry);
    }
    // Wrap the whole thing in one object under "strat_values"
    nlohmann::json result = nlohmann::json::array();
    result.push_back({{"strat_values", strat_values_array}});
    result.push_back({{"strat_perfs", strat_perfs_array}});
    return result;
}

nlohmann::json run_strategies(UserInputHandler &inputs)
{
    std::vector<struct GeneralParameters> global_params = inputs.get_general_parameters();
    std::vector<struct RecurrentInvestmentParameters> rinv_params = inputs.get_rinv_parameters();
    std::vector<struct RiskParameters> risk_params = inputs.get_risk_parameters();
    std::vector<struct TechnicalIndicators> techind_params = inputs.get_technical_indicators();
    assert(
        global_params.size() == rinv_params.size() &&
        rinv_params.size() == risk_params.size() &&
        risk_params.size() == techind_params.size() &&
        "Params don't have the same number of strategies \n"
    );
    std::vector<std::time_t> all_dates = inputs.get_all_tickers_dates();
    std::map<std::string, std::map<std::time_t, std::vector<double>>> strats_values;
    std::map<std::string, std::vector<double>> strats_perfs;
    for (size_t strat_i = 0; strat_i < global_params.size(); ++strat_i)
    {
        StrategyConfig config(global_params[strat_i], rinv_params[strat_i], risk_params[strat_i], techind_params[strat_i]);
        CustomStrategy *strat = new CustomStrategy(config);
        std::map<std::time_t, std::vector<double>> strat_values = strat->run_strategy();
        
        double total_returns = strat->get_strategy_total_returns();
        double xirr = strat->get_strategy_extended_internal_return_rate(1e-3, 1000);
        double total_investments = strat->get_strategy_total_investments();
        
        strats_values[global_params[strat_i].strategy_name] = strat_values;
        strats_perfs[global_params[strat_i].strategy_name] = {total_returns, xirr, total_investments};
        //strat->save_end_portfolio(config.global_params.strategy_name);
        //strat->run_montecarlo_simulations(1000);
        delete strat;
    }
    return merge_strats_json(all_dates, strats_values, strats_perfs);
}

nlohmann::json get_json_backtest_params(const nlohmann::json& client_json)
{
    nlohmann::json json_backtest_params;

    json_backtest_params["nb_strat"] = client_json["nbStrategy"];
    json_backtest_params["fees_per_trade"] = client_json["tradeFees"];
    json_backtest_params["flat_tax"] = client_json["tax"];
    json_backtest_params["strategy_name"] = client_json["portfolioName"];
    json_backtest_params["start_date"] = client_json["startDate"];
    json_backtest_params["end_date"] = client_json["endDate"];
    json_backtest_params["starting_amount"] = client_json["startingAmount"];
    json_backtest_params["monthly_deposit_amount"] = client_json["monthlyDeposit"];
    json_backtest_params["rinv_starting_amount"] = client_json["startingAmount"];
    json_backtest_params["rinv_investment_amount"] = client_json["rinvInvestmentAmount"];
    json_backtest_params["rinv_investment_nb_months_frequency"] = client_json["rinvInvestmentNbMonthsFrequency"];
    json_backtest_params["rinv_investment_montly_weeknum"] = client_json["rinvInvestmentMonthlyWeekNum"];
    json_backtest_params["rinv_investment_week_day"] = client_json["rinvInvestmentWeekDay"];
    json_backtest_params["rinv_rebalancing_threshold"] = client_json["rinvRebalancingThreshold"];
    json_backtest_params["rinv_rebalancing_freq_nb_day"] = client_json["rinvRebalancingFreqMinNbDays"];
    json_backtest_params["rinv_withdrawal_pct"] = client_json["rinvWithdrawalPct"];
    json_backtest_params["rinv_withdrawal_amount"] = client_json["rinvWithdrawalAmount"];
    json_backtest_params["rinv_withdrawal_nb_months_frequency"] = client_json["rinvWithdrawalNbMonthsFrequency"];
    json_backtest_params["rinv_withdrawal_monthly_weeknum"] = client_json["rinvWithdrawalMonthlyWeekNum"];
    json_backtest_params["rinv_withdrawal_week_day"] = client_json["rinvWithdrawalWeekDay"];
    json_backtest_params["rinv_rebalancing_threshold"] = client_json["rinvRebalancingThreshold"];
    json_backtest_params["reinvestment_policy"] = client_json["reinvestmentPolicy"];
    json_backtest_params["rinv_assets_desired_pct_allocations"] = client_json["rinvAllocations"];
    json_backtest_params["techind_sma_period"] = client_json["techindSmaWindow"];
    json_backtest_params["techind_rsi_period"] = client_json["techindRsiWindow"];
    json_backtest_params["techind_rsi_buy_threshold"] = client_json["techindRsiBuyThreshold"];
    json_backtest_params["techind_rsi_sell_threshold"] = client_json["techindRsiSellThreshold"];
    json_backtest_params["techind_rsi_tickers"] = client_json["assets"];
    json_backtest_params["techind_sma_tickers"] = client_json["assets"];
    json_backtest_params["techind_rsi_sma_tickers"] = client_json["assets"];
    json_backtest_params["rinv_tickers"] = client_json["assets"];
    json_backtest_params["all_tickers"] = client_json["assets"]; //TODO: Adapt the code accordingly to add risk, technical ind tickers

    return json_backtest_params;
}

std::string run_backtest(const nlohmann::json& client_json){
    nlohmann::json input_json = get_json_backtest_params(client_json);
    UserInputHandler *inputs = new UserInputHandler(input_json);
    nlohmann::json json_res = run_strategies(*inputs);
    delete inputs;
    return json_res.dump();
}

void run_server(){
    httplib::Server svr;

    svr.Options("/run-backtest", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 200;  // OK status for preflight
    });

    svr.Post("/run-backtest", [](const httplib::Request& req, httplib::Response& res) {
        // Parse JSON request body
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");

        auto json_client = nlohmann::json::parse(req.body);
        // Run backtest with received parameters
        //std::cout << "Received request: " << req.body << std::endl;
        std::string result = run_backtest(json_client);
        //std::cout << "Backtest result: " << result << std::endl;
        // Respond to frontend
        res.set_content(result, "application/json");
        res.set_header("Content-Length", std::to_string(result.size())); // Explicitly set content length
        res.set_header("Connection", "close");  // Ensure connection closes after sending full response
    });

    std::cout << "Server listening on port 8080..." << std::endl;
    svr.listen("127.0.0.1", 8080);
}

/*
// Function to run the backtest with parameters
std::string run_backtest(const nlohmann::json& client_json) {
    
    nlohmann::json json_backtest_params = get_json_backtest_params(client_json);
    // Construct the command with the cleaned arguments
    std::ostringstream command;
    command << "./main --nb_strat=" << json_backtest_params["nb_strat"] << " "  
            << "--fees_per_trade="<< json_backtest_params["fees_per_trade"] << " "  
            << "--flat_tax=" << json_backtest_params["flat_tax"] << " "
            << "--strategy_name=" << json_backtest_params["portfolio_name"] << " "
            << "--all_tickers=" << json_backtest_params["all_tickers"] << " "
            << "--start_date=" <<  json_backtest_params["start_date"] << " "
            << "--end_date=" << json_backtest_params["end_date"] << " "
            << "--starting_amount=" << json_backtest_params["starting_amount"] << " "
            << "--monthly_deposit_amount=" << json_backtest_params["monthly_deposit_amount"] << " "
            << "--reinvestment_policy=" << json_backtest_params["reinvestment_policy"] << " "
            << "--rinv_tickers=" << json_backtest_params["rinv_tickers"] << " "
            << "--rinv_starting_amount=" << json_backtest_params["starting_amount"] << " "
            << "--rinv_investment_amount=" << json_backtest_params["monthly_deposit_amount"] << " "
            << "--rinv_investment_nb_months_frequency=" << json_backtest_params["rinv_investment_nb_months_frequency"] << " "
            << "--rinv_investment_montly_weeknum=" << json_backtest_params["rinv_investment_montly_weeknum"]  << " "
            << "--rinv_investment_week_day=" << json_backtest_params["rinv_investment_week_day"]  << " "
            << "--rinv_rebalancing_threshold=" << json_backtest_params["rinv_rebalancing_threshold"]  << " "
            << "--rinv_rebalancing_freq_nb_day=" << json_backtest_params["rinv_rebalancing_freq_nb_day"]  << " "
            << "--rinv_withdrawal_pct=" << json_backtest_params["rinv_withdrawal_pct"] << " "
            << "--rinv_withdrawal_nb_months_frequency=" << json_backtest_params["rinv_withdrawal_nb_months_frequency"] << " "
            << "--rinv_withdrawal_monthly_weeknum=" << json_backtest_params["rinv_withdrawal_monthly_weeknum"] << " "
            << "--rinv_withdrawal_week_day=" << json_backtest_params["rinv_withdrawal_week_day"] << " "
            << "--rinv_assets_desired_pct_allocations=" << json_backtest_params["rinv_assets_desired_pct_allocations"];

    std::cout << "Executing command: " << command.str() << std::endl;

    FILE* pipe = popen(command.str().c_str(), "r");  // Run the command
    if (!pipe) return "{\"error\": \"Failed to execute backtest\"}";

    char buffer[256];
    std::ostringstream output;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output << buffer;
    }
    pclose(pipe);

    std::cout << output.str() << std::endl;
    return output.str();
}*/