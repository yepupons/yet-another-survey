#include "database.hpp"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/json.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <set>
#include <stdexcept>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

namespace survey {
std::string Database::read_survey(int survey_id) {
    auto result = db()["surveys"].find_one(
        document{} << "data.id" << survey_id << finalize
    );
    if (result) {
        return bsoncxx::to_json(result->view());
    }
    throw std::runtime_error("Survey not found");
}

void Database::write_survey(const std::string &survey_data) {
    bsoncxx::document::value doc = bsoncxx::from_json(survey_data);
    db()["surveys"].insert_one(doc.view());
}

/*
std::string Database::read_answer(int answer_id) {
    auto result = db()["answers"].find_one(document{} << "data.id" << answer_id
<< finalize); if (result) { return bsoncxx::to_json(result->view());
    }
    throw std::runtime_error("Answer not found");
}
*/

void Database::write_answer(const std::string &answer_data) {
    bsoncxx::document::value doc = bsoncxx::from_json(answer_data);
    db()["answers"].insert_one(doc.view());
}

std::string Database::read_passed_surveys(int session_id) {
    mongocxx::options::find opts;
    opts.projection(
        bsoncxx::builder::stream::document{}
        << "data.survey_id" << 1 << "_id" << 0
        << bsoncxx::builder::stream::finalize
    );
    auto cursor = db()["answers"].find(
        document{} << "data.respondent_id" << session_id << finalize, opts
    );
    std::set<int> passed_surveys;
    for (auto &&doc : cursor) {
        if (doc["data"] && doc["data"]["survey_id"]) {
            passed_surveys.insert(doc["data"]["survey_id"].get_int32().value);
        }
    }
    nlohmann::json result = nlohmann::json::array();
    for (int id : passed_surveys) {
        result.push_back(id);
    }
    return result.dump();
}
}  // namespace survey
