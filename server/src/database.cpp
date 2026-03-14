#include "database.hpp"

std::filesystem::path Database::resolve_public_root() {
    auto pb_root = std::filesystem::current_path() / "public";
    if (!std::filesystem::exists(pb_root)) {
        pb_root = std::filesystem::current_path() / ".." / "public";
    }
    return pb_root;
}

std::filesystem::path Database::resolve_database_root() {
    auto db_root = std::filesystem::current_path() / "database";
    if (!std::filesystem::exists(db_root)) {
        db_root = std::filesystem::current_path() / ".." / "database";
    }
    return db_root;
}

void Database::write_response(const nlohmann::json &response, int survey_id) {
    auto now = std::chrono::system_clock::now();
    auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()
    )
                  .count();
    // maybe we should add hash to it, but later D:
    std::string filename = std::to_string(ts) + ".json";
    auto root = resolve_public_root();
    std::filesystem::path base = root / std::to_string(survey_id) / "responses";
    std::error_code ec;
    std::filesystem::create_directories(base, ec);
    if (ec) {
        throw std::runtime_error("Failed to create responses directory");
    }
    std::filesystem::path out_path = base / filename;

    std::ofstream out(out_path);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to write response");
    }

    auto db_route = resolve_database_root();
    std::ifstream db_out(db_route / "passed_surveys.json");
    nlohmann::json db_json;
    if (db_out.is_open()) {
        db_out >> db_json;
    } else {
        db_json = nlohmann::json::array();
    }
    std::string session_id =
        response["answer_data"]["user_id"].get<std::string>();
    if (!db_json.contains(session_id)) {
        db_json[session_id] = nlohmann::json::array();
    }
    db_json[session_id].push_back({survey_id, filename});

    std::ofstream o(db_route / "passed_surveys.json");
    o << std::setw(4) << db_json << std::endl;

    out << response.dump(2) << std::endl;
}

void Database::register_test(
    const std::string &survey_id,
    const nlohmann::json &test_json
) {
    auto now = std::chrono::system_clock::now();
    auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()
    )
                  .count();
    if (!std::filesystem::exists(std::filesystem::current_path() / "public")) {
        throw std::runtime_error("Public directory does not exist");
        return;
    }

    auto base = resolve_public_root() / survey_id / "survey";
    std::error_code ec;
    std::filesystem::create_directories(base, ec);
    if (ec) {
        throw std::runtime_error("Failed to create survey directory");
    }

    std::ofstream out(base / "data.json");
    if (!out.is_open()) {
        throw std::runtime_error("Failed to write survey data");
    }
    out << test_json.dump(2) << std::endl;

    std::filesystem::create_directories(
        resolve_public_root() / survey_id / "responses", ec
    );
    if (ec) {
        throw std::runtime_error("Failed to create responses directory");
    }
}

nlohmann::json Database::send_passed_surveys(const std::string &session_id) {
    auto db_route = resolve_database_root();
    std::ifstream db_out(db_route / "passed_surveys.json");
    nlohmann::json db_json;
    if (db_out.is_open()) {
        db_out >> db_json;
    } else {
        throw std::runtime_error("Failed to read passed surveys database");
    }
    if (!db_json.contains(session_id)) {
        throw std::runtime_error("Session ID not found in database");
    }
    auto passed_surveys = db_json[session_id];
    return passed_surveys;
}
