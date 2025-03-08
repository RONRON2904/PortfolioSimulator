#include <nlohmann/json.hpp>
#include "../headers/input_handler.hpp"
#include "../headers/yahoo_utils.hpp"
#include <fmt/core.h>

UserInputHandler::UserInputHandler(int argc, char* argv[])
{
    nlohmann::json allowed_params = read_json_file("../allowed_params.json");
    std::set<std::string> general_params_allowed_list = allowed_params["GeneralParameters"];
    std::set<std::string> rinv_params_allowed_list = allowed_params["RecurrentInvestmentParameters"];
    std::set<std::string> risk_params_allowed_list = allowed_params["RiskParameters"];
    std::set<std::string> techind_params_allowed_list = allowed_params["TechnicalIndicators"];
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        size_t pos = arg.find('=');
        bool correct_param = false;
        if (pos != std::string::npos) {
            std::string key = arg.substr(0, pos);
            std::string value = arg.substr(pos + 1);
            if (general_params_allowed_list.find(key) == general_params_allowed_list.end())
            {
                this->general_param_args[key] = value;
                correct_param = true;
            }
            else if (rinv_param_args.find(key) == rinv_param_args.end())
            {
                this->rinv_param_args[key] = value;
                correct_param = true;
            }
            else if (risk_param_args.find(key) == risk_param_args.end())
            {
                this->risk_param_args[key] = value;
                correct_param = true;
            }
            else if (techind_param_args.find(key) == techind_param_args.end())
            {
                this->techind_param_args[key] = value;
                correct_param = true;
            }
        }
        assert (correct_param && fmt::format("Parameter {} is not recognized", argv[i]).c_str());
    }

    struct GeneralParameters& set_general_parameters()
    {
        
        GeneralParameters general_params = {general_param_args, "CustomRSISMA_2015_2025", 120000.0, 2500.0, 1.0, 0.3, true};
    }
};