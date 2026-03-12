#pragma once
#include <drogon/HttpController.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

class Database {
public:
    static void write_response(const nlohmann::json &response, int survey_id);
    static void register_test(
        const std::string &survey_id,
        const nlohmann::json &test_json
    );
    static std::filesystem::path resolve_public_root();
};
