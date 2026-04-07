#include "database.hpp"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/json.hpp>
#include <nlohmann/json.hpp>
#include <set>
#include <stdexcept>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

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
    auto insert_result = db()["surveys"].insert_one(doc.view());
    if (!insert_result) {
        throw std::runtime_error("Writing survey into database failed");
    }

    int survey_id = doc.view()["data"]["id"].get_int32().value;
    int creator_id = doc.view()["data"]["creator_id"].get_int32().value;

    auto find_result =
        db()["users"].find_one(document{} << "id" << creator_id << finalize);
    if (!find_result) {
        db()["users"].insert_one(
            document{} << "id" << creator_id << "created_surveys" << open_array
                       << close_array << "given_answers" << open_array
                       << close_array << finalize
        );
    }
    db()["users"].update_one(
        document{} << "id" << creator_id << finalize,
        document{} << "$push" << open_document << "created_surveys" << survey_id
                   << close_document << finalize
    );
}

/*
std::string Database::read_answer(int answer_id) {
    auto result = db()["answers"].find_one(
        document{} << "data.id" << answer_id << finalize
    );
    if (result) {
        return bsoncxx::to_json(result->view());
    }
    throw std::runtime_error("Answer not found");
}
*/

void Database::write_answer(const std::string &answer_data) {
    bsoncxx::document::value doc = bsoncxx::from_json(answer_data);
    auto insert_result = db()["answers"].insert_one(doc.view());
    if (!insert_result) {
        throw std::runtime_error("Writing answer into database failed");
    }

    int answer_id = doc.view()["data"]["id"].get_int32().value;
    int respondent_id = doc.view()["data"]["respondent_id"].get_int32().value;

    auto find_result =
        db()["users"].find_one(document{} << "id" << respondent_id << finalize);
    if (!find_result) {
        db()["users"].insert_one(
            document{} << "id" << respondent_id << "created_surveys"
                       << open_array << close_array << "given_answers"
                       << open_array << close_array << finalize
        );
    }
    db()["users"].update_one(
        document{} << "id" << respondent_id << finalize,
        document{} << "$push" << open_document << "given_answers" << answer_id
                   << close_document << finalize
    );
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
