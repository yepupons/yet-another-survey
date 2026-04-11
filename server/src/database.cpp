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
    mongocxx::options::find opts;
    opts.projection(document{} << "sections.questions.answer" << 0 << finalize);
    auto result = db()["surveys"].find_one(
        document{} << "data.id" << survey_id << finalize, opts
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

std::string Database::read_survey_results(int session_id, int survey_id) {
    mongocxx::options::find opts;
    opts.projection(
        bsoncxx::builder::stream::document{}
        << "data.survey_id" << 1 << "sections" << 1 << "_id" << 0
        << bsoncxx::builder::stream::finalize
    );
    auto cursor = db()["answers"].find(
        document{} << "data.respondent_id" << session_id << "data.survey_id"
                   << survey_id << finalize,
        opts
    );
    nlohmann::json results = nlohmann::json::array();
    for (auto &&doc : cursor) {
        if (doc["sections"]) {
            results.push_back(nlohmann::json::parse(
                bsoncxx::to_json(doc["sections"].get_array().value)
            ));
        }
    }
    return results.dump();
}

std::string Database::get_result(const std::string &user_result_data) {
    Database::write_answer(user_result_data);
    nlohmann::json user_json = nlohmann::json::parse(user_result_data);
    int survey_id = user_json.at("data").at("survey_id").get<int>();
    mongocxx::options::find opts;
    opts.projection(
        document{} << "sections.questions.answer" << 1
                   << "sections.questions.type" << 1 << "_id" << 0 << finalize
    );
    auto result = db()["surveys"].find_one(
        document{} << "data.id" << survey_id << finalize, opts
    );

    nlohmann::json survey_json =
        nlohmann::json::parse(bsoncxx::to_json(result->view()));
    nlohmann::json out;
    out["data"] = {
        {"survey_id", survey_id},
    };
    out["sections"] = nlohmann::json::array();
    int question_amount = 0;
    int correct_answers = 0;
    for (size_t section_indx = 0; section_indx < survey_json["sections"].size();
         section_indx++) {
        nlohmann::json section_result = nlohmann::json::array();
        for (size_t question_indx = 0;
             question_indx <
             survey_json["sections"].at(section_indx)["questions"].size();
             question_indx++) {
            question_amount++;
            auto question =
                survey_json["sections"].at(section_indx)["questions"].at(
                    question_indx
                );
            std::string type = question.at("type");
            auto correct_answer = question.at("answer");
            auto user_answer =
                user_json["sections"].at(section_indx).at(question_indx);

            if (type == "single") {
                // 1 correct, 0 isnt
                int status =
                    correct_answer.get<int>() == user_answer.get<int>();
                correct_answers += status;
                section_result.emplace_back(status);
            } else if (type == "multiple") {
                std::set<int> user_set;
                std::set<int> correct_set;
                for (auto &v : user_answer) {
                    user_set.insert(v.get<int>());
                }
                for (auto &v : correct_answer) {
                    correct_set.insert(v.get<int>());
                }
                int status = user_set == correct_set;
                correct_answers += status;
                section_result.emplace_back(status);
            } else if (type == "text") {
                if (correct_answer.is_string()) {
                    section_result.emplace_back(
                        user_answer.get<std::string>() ==
                        correct_answer.get<std::string>()
                    );
                } else if (correct_answer.is_array()) {
                    bool ok = false;
                    for (auto &v : correct_answer) {
                        if (user_answer.get<std::string>() ==
                            v.get<std::string>()) {
                            section_result.emplace_back(1);
                            ok = true;
                            correct_answers++;
                            break;
                        }
                    }
                    if (!ok) {
                        section_result.emplace_back(0);
                    }
                }
            }
        }
        out["sections"].push_back(section_result);
    }
    out["data"]["question_amount"] = question_amount;
    out["data"]["correct_answers"] = correct_answers;
    return out.dump();
}
}  // namespace survey
