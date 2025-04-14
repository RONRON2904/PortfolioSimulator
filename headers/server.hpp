#ifndef SERVER_HPP
#define SERVER_HPP

#include "../headers/input_handler.hpp"

nlohmann::json merge_strats_json(const std::vector<std::time_t> &all_dates, 
                                 const std::map<std::string, std::map<std::time_t, std::vector<double>>> strats_values, 
                                 const std::map<std::string, std::vector<double>> strats_perfs);
nlohmann::json run_strategies(UserInputHandler &inputs);
nlohmann::json get_json_backtest_params(const nlohmann::json& client_json);
std::string run_backtest(const nlohmann::json& client_json);
void run_server();

#endif