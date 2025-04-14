#include "../headers/yahoo_finance.hpp"
#include "../headers/strategy.hpp"
#include "../headers/input_handler.hpp"
#include "../headers/yahoo_utils.hpp"
#include "../headers/server.hpp"

#include <iostream>

// g++ -g -fopenmp server.cpp yahoo_*.cpp strateg*.cpp input_handler.cpp portfolio_builder.cpp main.cpp -o main -lcurl -lmpi -lfmt
// g++ -fopenmp *.cpp -o main -lcurl -lmpi for parallelized version

int main(){
    run_server();
    return EXIT_SUCCESS;
}

/*
nlohmann::json merge_strats_json(std::vector<std::time_t> &all_dates, std::map<std::string, std::map<std::time_t, std::vector<double>>> strats_values){
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
    for (size_t strat_i = 0; strat_i < global_params.size(); ++strat_i)
    {
        StrategyConfig config(global_params[strat_i], rinv_params[strat_i], risk_params[strat_i], techind_params[strat_i]);
        CustomStrategy *strat = new CustomStrategy(config);
        std::map<std::time_t, std::vector<double>> strat_values = strat->run_strategy();
        strats_values[global_params[strat_i].strategy_name] = strat_values;

        //strat->save_end_portfolio(config.global_params.strategy_name);
        //strat->run_montecarlo_simulations(1000);
        delete strat;
    }
    return merge_strats_json(all_dates, strats_values);
}

int main(int argc, char* argv[])
{
    UserInputHandler *inputs = new UserInputHandler(argc, argv);
    nlohmann::json json_res = run_strategies(*inputs);
    std::cout << json_res.dump() << std::endl;

    delete inputs;
    return EXIT_SUCCESS;
}*/
