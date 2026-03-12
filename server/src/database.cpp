#include "database.hpp"

std::filesystem::path Database::resolve_public_root() {
    auto root = std::filesystem::current_path() / "public";
    if (!std::filesystem::exists(root)) {
        root = std::filesystem::current_path() / ".." / "public";
    }
    return root;
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
