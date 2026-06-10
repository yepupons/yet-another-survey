#include "survey_service.hpp"
#include "survey_validator.hpp"
#include "database.hpp"
#include <algorithm>
#include <functional>
#include <set>
#include <nlohmann/json.hpp>
#include <chrono>
#include <ctime>

namespace survey {
static std::string time_now() {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&currentTime);
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%d.%m.%Y %H:%M:%S", localTime);
    return buffer;
}

SurveyService::SurveyService(Database &db) : db_(db) {}

std::string SurveyService::create_survey(const std::string &access_token, const nlohmann::json &payload){
    const std::string creator_id = db_.user_id_by_access_token(access_token);
    const std::string survey_id = survey::Database::generate_uuid();
    validator_.validate_survey_payload(payload);
    nlohmann::json survey_json = payload;
    survey_json["data"]["id"] = survey_id;
    survey_json["data"]["creator_id"] = creator_id;
    if (!survey_json["data"].contains("is_public")) {
        survey_json["data"]["is_public"] = true;
    }
    if (!survey_json["data"].contains("preview_image")) {
        const int index = std::hash<std::string>{}(survey_id) % 6;
        survey_json["data"]["preview_image"] = "default:" + std::to_string(index);
    }
    survey_json["data"]["likes_count"] = 0;
    survey_json["data"]["dislikes_count"] = 0;
    survey_json["data"]["ratings_count"] = 0;
    survey_json["data"]["rating_score"] = 0;
    return db_.write_survey(survey_json);
}

std::string SurveyService::submit_answer(
    const std::string &access_token,
    const nlohmann::json &payload,
    const std::string &survey_id
){
    const std::string respondent_id = db_.user_id_by_access_token(access_token);
    const std::string answer_id = survey::Database::generate_uuid();
    validator_.validate_answer_payload(nlohmann::json::parse(db_.read_survey(survey_id)), payload);
    nlohmann::json answer_json = payload;
    answer_json["data"]["id"] = answer_id;
    answer_json["data"]["survey_id"] = survey_id;
    answer_json["data"]["respondent_id"] = respondent_id;
    answer_json["data"]["completed_at"] = time_now();
    return db_.write_answer(answer_json);
}

std::string SurveyService::check_answer(
    const std::string &access_token,
    const nlohmann::json &payload
) {
    const std::string respondent_id = db_.user_id_by_access_token(access_token);
    const std::string answer_id = survey::Database::generate_uuid();
    const std::string survey_id =
        payload.at("data").at("survey_id").get<std::string>();

    const nlohmann::json survey_json =
        nlohmann::json::parse(db_.read_survey_with_answers(survey_id));
    validator_.validate_answer_payload(survey_json, payload);

    nlohmann::json answer_json = payload;
    answer_json["data"]["id"] = answer_id;
    answer_json["data"]["survey_id"] = survey_id;
    answer_json["data"]["respondent_id"] = respondent_id;
    answer_json["data"]["completed_at"] = time_now();
    db_.write_answer(answer_json);

    nlohmann::json out;
    out["data"] = {
        {"answer_id", answer_id},
        {"survey_id", survey_id},
    };
    out["sections"] = nlohmann::json::array();

    int question_amount = 0;
    int correct_answers = 0;
    for (std::size_t section_index = 0;
         section_index < survey_json.at("sections").size(); ++section_index) {
        const auto &user_section =
            answer_json.at("sections").at(section_index);
        if (user_section.empty()) {
            out["sections"].push_back(nlohmann::json::array());
            continue;
        }

        nlohmann::json section_result = nlohmann::json::array();
        const auto &questions =
            survey_json.at("sections").at(section_index).at("questions");
        for (std::size_t question_index = 0; question_index < questions.size();
             ++question_index) {
            ++question_amount;
            const auto &question = questions.at(question_index);
            const std::string type = question.at("type").get<std::string>();
            const auto &correct_answer = question.at("answer");
            const auto &user_answer =
                user_section.at(question_index).at("answer");

            if (type == "single" || type == "multiple") {
                if (correct_answer == user_answer) {
                    ++correct_answers;
                    section_result.push_back(1);
                } else {
                    section_result.push_back(0);
                }
            } else if (type == "text") {
                if (std::find(
                        correct_answer.begin(), correct_answer.end(),
                        user_answer
                    ) != correct_answer.end()) {
                    ++correct_answers;
                    section_result.push_back(1);
                } else {
                    section_result.push_back(0);
                }
            }
        }
        out["sections"].push_back(section_result);
    }

    out["data"]["question_amount"] = question_amount;
    out["data"]["correct_answers"] = correct_answers;
    return out.dump();
}
}
