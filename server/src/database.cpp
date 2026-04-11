#include "database.hpp"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/json.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <set>
#include <stdexcept>
#include <string>

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

std::string Database::read_created_surveys(int session_id) {
    mongocxx::options::find opts;
    opts.projection(
        bsoncxx::builder::stream::document{}
        << "created_surveys" << 1 << "_id" << 0
        << bsoncxx::builder::stream::finalize
    );
    auto result = db()["users"].find_one(
        document{} << "id" << session_id << finalize, opts
    );
    if (result) {
        return bsoncxx::to_json(
            result->view()["created_surveys"].get_array().value
        );
    }
    throw std::runtime_error("User not found");
}

std::string Database::read_statistics(int survey_id) {
    auto survey = db()["surveys"].find_one(
        document{} << "data.id" << survey_id << finalize
    );
    if (!survey) {
        throw std::runtime_error("Survey not found");
    }
    nlohmann::json survey_data =
        nlohmann::json::parse(bsoncxx::to_json(survey->view()));
    nlohmann::json result;
    result["total_answers"] = db()["answers"].count_documents(
        document{} << "data.survey_id" << survey_id << finalize
    );
    result["sections"] = nlohmann::json::array();
    for (int i = 0; i < survey_data["sections"].size(); ++i) {
        result["sections"].push_back(nlohmann::json::array());
        for (int j = 0; j < survey_data["sections"][i]["questions"].size();
             ++j) {
            result["sections"].back().push_back(nlohmann::json::object());
        }
    }

    mongocxx::pipeline p{};
    p.match(document{} << "data.survey_id" << survey_id << finalize);
    p.unwind(
        document{} << "path" << "$sections" << "includeArrayIndex" << "section"
                   << finalize
    );
    p.unwind(
        document{} << "path" << "$sections" << "includeArrayIndex" << "question"
                   << finalize
    );
    p.unwind(document{} << "path" << "$sections.answer" << finalize);
    p.match(
        document{} << "sections.answer" << open_document << "$ne" << ""
                   << close_document << finalize
    );
    p.group(
        document{} << "_id"
                   << (document{} << "section" << "$section" << "question"
                                  << "$question" << "answer"
                                  << "$sections.answer" << finalize)
                   << "count" << (document{} << "$sum" << 1 << finalize)
                   << finalize
    );
    auto cursor = db()["answers"].aggregate(p);

    for (auto &&doc : cursor) {
        auto el = doc["_id"];
        int section = el["section"].get_int64();
        int question = el["question"].get_int64();
        std::string answer;
        switch (el["answer"].type()) {
            case bsoncxx::type::k_string:
                answer = el["answer"].get_string().value.data();
                break;
            case bsoncxx::type::k_int32:
                answer = std::to_string(el["answer"].get_int32().value);
                break;
            default:
                continue;
        }
        result.at("sections")[section][question][answer] =
            doc["count"].get_int32().value;
    }
    return result.dump();
}
}  // namespace survey
