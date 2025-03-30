#include <iostream>
#include <httplib.h>
#include <cstdlib>
#include <sstream>  
#include <nlohmann/json.hpp>

// g++ -o server server.cpp

nlohmann::json getJsonBacktestParams(const nlohmann::json& client_json)
{
    nlohmann::json json_backtest_params;

    json_backtest_params["nb_strat"] = client_json["nbStrategy"];
    json_backtest_params["fees_per_trade"] = client_json["tradeFees"];
    json_backtest_params["flat_tax"] = client_json["tax"];
    json_backtest_params["portfolio_name"] = client_json["portfolioName"];
    json_backtest_params["start_date"] = client_json["startDate"];
    json_backtest_params["end_date"] = client_json["endDate"];
    json_backtest_params["starting_amount"] = client_json["startingAmount"];
    json_backtest_params["monthly_deposit_amount"] = client_json["monthlyDeposit"];
    json_backtest_params["rinv_starting_amount"] = client_json["rinvInvestmentAmount"];
    json_backtest_params["rinv_investment_nb_months_frequency"] = client_json["rinvInvestmentNbMonthsFrequency"];
    json_backtest_params["rinv_investment_montly_weeknum"] = client_json["rinvInvestmentMonthlyWeekNum"];
    json_backtest_params["rinv_investment_week_day"] = client_json["rinvInvestmentWeekDay"];
    json_backtest_params["rinv_rebalancing_threshold"] = client_json["rinvRebalancingThreshold"];
    json_backtest_params["rinv_rebalancing_freq_nb_day"] = client_json["rinvRebalancingFreqMinNbDays"];
    json_backtest_params["rinv_withdrawal_pct"] = client_json["rinvWithdrawalPct"];
    json_backtest_params["rinv_withdrawal_nb_months_frequency"] = client_json["rinvWithdrawalNbMonthsFrequency"];
    json_backtest_params["rinv_withdrawal_monthly_weeknum"] = client_json["rinvWithdrawalMonthlyWeekNum"];
    json_backtest_params["rinv_withdrawal_week_day"] = client_json["rinvWithdrawalWeekDay"];
    json_backtest_params["rinv_rebalancing_threshold"] = client_json["rinvRebalancingThreshold"];
    json_backtest_params["reinvestment_policy"] = client_json["reinvestmentPolicy"];
    std::cout << json_backtest_params << std::endl;

    json_backtest_params["rinv_assets_desired_pct_allocations"] = client_json["rinvAllocations"];
    json_backtest_params["rinv_tickers"] = client_json["assets"];
    json_backtest_params["all_tickers"] = client_json["assets"]; //TODO: Adapt the code accordingly to add risk, technical ind tickers

    std::cout << json_backtest_params << std::endl;

    return json_backtest_params;
}

// Function to run the backtest with parameters
std::string runBacktest(const nlohmann::json& client_json) {
    
    nlohmann::json json_backtest_params = getJsonBacktestParams(client_json);
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
}

int main() {
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
        std::string result = runBacktest(json_client);

        // Respond to frontend
        res.set_content(result, "application/json");
        res.set_header("Content-Length", std::to_string(result.size())); // Explicitly set content length
        res.set_header("Connection", "close");  // Ensure connection closes after sending full response
    });

    std::cout << "Server listening on port 8080..." << std::endl;
    svr.listen("127.0.0.1", 8080);
}
