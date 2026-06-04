#include "survey_service.hpp"
#include "database.hpp"
#include <algorithm>
#include <set>
#include <nlohmann/json.hpp>

namespace survey {
SurveyService::SurveyService(Database &db) : db_(db) {}

void SurveyService::validate_survey_payload(const nlohmann::json &payload) const {
    if (!payload.contains("data") || !payload["data"].is_object()) {
        throw std::invalid_argument("Survey data is required");
    }

    if (!payload["data"].contains("type") || !payload["data"]["type"].is_string()) {
        throw std::invalid_argument("Survey type is required");
    }

    const std::string type = payload["data"]["type"].get<std::string>();
    if (type != "survey" && type != "test" && type != "quiz") {
        throw std::invalid_argument("Invalid survey type");
    }

    if (payload["data"].contains("is_public") &&
        !payload["data"]["is_public"].is_boolean()) {
        throw std::invalid_argument("Survey visibility must be boolean");
    }

    if (!payload.contains("title") || !payload["title"].is_string()) {
        throw std::invalid_argument("Survey title is required");
    }

    const std::string title = payload["title"].get<std::string>();
    if (title.empty() || title.size() > 200) {
        throw std::invalid_argument("Invalid survey title");
    }

    if (!payload.contains("sections") || !payload["sections"].is_array()) {
        throw std::invalid_argument("Survey sections are required");
    }

    const auto &sections = payload["sections"];
    if (sections.empty() || sections.size() > 50) {
        throw std::invalid_argument("Invalid sections count");
    }

    for (const auto &section : sections) {
        if (!section.contains("questions") || !section["questions"].is_array()) {
            throw std::invalid_argument("Section questions are required");
        }

        if (section["questions"].empty() || section["questions"].size() > 100) {
            throw std::invalid_argument("Invalid questions count");
        }

        for (const auto &question : section["questions"]) {
            validate_question_payload(type, question);
        }
    }
}

void SurveyService::validate_question_payload(
    const std::string &survey_type,
    const nlohmann::json &question
) const {
    if (!question.contains("type") || !question["type"].is_string()) {
        throw std::invalid_argument("Question type is required");
    }

    const std::string type = question["type"].get<std::string>();
    if (type != "text" && type != "single" && type != "multiple") {
        throw std::invalid_argument("Invalid question type");
    }

    if (!question.contains("text") || !question["text"].is_string()) {
        throw std::invalid_argument("Question text is required");
    }

    if (question["text"].get<std::string>().size() > 3000) {
        throw std::invalid_argument("Question text is too long");
    }

    if (question.contains("required") && !question["required"].is_boolean()) {
        throw std::invalid_argument("Question required must be boolean");
    }

    if (question.contains("image") && !question["image"].is_string()) {
        throw std::invalid_argument("Question image must be string");
    }

    if (type == "single" || type == "multiple") {
        if (!question.contains("options") || !question["options"].is_array()) {
            throw std::invalid_argument("Question options are required");
        }

        const auto &options = question["options"];
        if (options.size() < 1 || options.size() > 50) {
            throw std::invalid_argument("Invalid options count");
        }

        for (const auto &option : options) {
            if (!option.is_string() || option.get<std::string>().size() > 300) {
                throw std::invalid_argument("Invalid option");
            }
        }
    }

    if (survey_type == "test") {
        if (!question.contains("answer")) {
            throw std::invalid_argument("Correct answer is required for test");
        }
    }

    if (survey_type == "quiz") {
        if (!question.contains("scores") || !question["scores"].is_array()) {
            throw std::invalid_argument("Scores are required for quiz");
        }
    }
}

void SurveyService::validate_answer_payload(
    const nlohmann::json &survey,
    const nlohmann::json &answer
) const {
    if (!answer.contains("sections") || !answer["sections"].is_array()) {
        throw std::invalid_argument("Answer sections are required");
    }
    const auto &survey_sections = survey.at("sections");
    const auto &answer_sections = answer.at("sections");
    if (answer_sections.size() != survey_sections.size()) {
        throw std::invalid_argument("Invalid answer sections count");
    }

    for (std::size_t section_index = 0; section_index < survey_sections.size(); section_index++) {
        const auto &survey_section = survey_sections.at(section_index);
        const auto &answer_section = answer_sections.at(section_index);
        if (!survey_section.contains("questions") ||
            !survey_section.at("questions").is_array()) {
            throw std::invalid_argument("Invalid survey section");
        }
        if (!answer_section.is_array()) {
            throw std::invalid_argument("Answer section must be an array");
        }

        if (answer_section.empty()) {
            continue;
        }

        const auto &questions = survey_section.at("questions");
        if (answer_section.size() != questions.size()) {
            throw std::invalid_argument("Invalid answers count in section");
        }

        for (std::size_t question_index = 0; question_index < questions.size();
             question_index++) {
            const auto &question = questions.at(question_index);
            const auto &answer_item = answer_section.at(question_index);

            if (!question.contains("type") || !question.at("type").is_string()) {
                throw std::invalid_argument("Question type is required");
            }
            if (!answer_item.is_object() || !answer_item.contains("answer")) {
                throw std::invalid_argument("Answer item is invalid");
            }

            const std::string question_type = question.at("type").get<std::string>();
            const bool required = question.value("required", false);
            const auto &value = answer_item.at("answer");

            if (question_type == "text") {
                if (!value.is_string()) {
                    throw std::invalid_argument("Text answer must be string");
                }
                const std::string text_answer = value.get<std::string>();
                if (required && text_answer.empty()) {
                    throw std::invalid_argument("Required answer is empty");
                }
                if (text_answer.size() > 5000) {
                    throw std::invalid_argument("Text answer is too long");
                }
            } else if (question_type == "single") {
                if (!question.contains("options") ||
                    !question.at("options").is_array()) {
                    throw std::invalid_argument("Single question options are required");
                }
                if (!value.is_number_integer()) {
                    throw std::invalid_argument("Single answer must be integer");
                }

                const int selected = value.get<int>();
                const int options_count = question.at("options").size();
                if (selected == -1 && !required) {
                    continue;
                }
                if (selected < 1 || selected > options_count) {
                    throw std::invalid_argument("Single answer is out of range");
                }
            } else if (question_type == "multiple") {
                if (!question.contains("options") ||
                    !question.at("options").is_array()) {
                    throw std::invalid_argument("Multiple question options are required");
                }
                if (!value.is_array()) {
                    throw std::invalid_argument("Multiple answer must be array");
                }
                if (required && value.empty()) {
                    throw std::invalid_argument("Required answer is empty");
                }

                const int options_count = question.at("options").size();
                std::set<int> selected_options;
                for (const auto &selected_json : value) {
                    if (!selected_json.is_number_integer()) {
                        throw std::invalid_argument(
                            "Multiple answer option must be integer"
                        );
                    }
                    const int selected = selected_json.get<int>();
                    if (selected < 1 || selected > options_count) {
                        throw std::invalid_argument(
                            "Multiple answer is out of range"
                        );
                    }
                    if (!selected_options.insert(selected).second) {
                        throw std::invalid_argument(
                            "Multiple answer contains duplicates"
                        );
                    }
                }
            } else {
                throw std::invalid_argument("Invalid question type");
            }
        }
    }
}


std::string SurveyService::create_survey(const std::string &access_token, const nlohmann::json &payload){
    const std::string creator_id = db_.user_id_by_access_token(access_token);
    const std::string survey_id = survey::Database::generate_uuid();
    validate_survey_payload(payload);
    nlohmann::json survey_json = payload;
    survey_json["data"]["id"] = survey_id;
    survey_json["data"]["creator_id"] = creator_id;
    if (!survey_json["data"].contains("is_public")) {
        survey_json["data"]["is_public"] = true;
    }
    survey_json["data"]["likes_count"] = 0;
    survey_json["data"]["dislikes_count"] = 0;
    survey_json["data"]["ratings_count"] = 0;
    survey_json["data"]["rating_score"] = 0;
    return db_.write_survey(survey_id, creator_id, survey_json);
}

std::string SurveyService::submit_answer(
        const std::string &access_token,
        const nlohmann::json &payload,
        const std::string &survey_id
    ){
        const std::string respondent_id = db_.user_id_by_access_token(access_token);
        const std::string answer_id = survey::Database::generate_uuid();
        validate_answer_payload(nlohmann::json::parse(db_.read_survey(survey_id)), payload);
        nlohmann::json answer_json = payload;
        answer_json["data"]["survey_id"] = survey_id;
        auto out = db_.write_answer(answer_id, respondent_id, answer_json.dump());
        return out;
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
    validate_answer_payload(survey_json, payload);

    nlohmann::json answer_json = payload;
    answer_json["data"]["survey_id"] = survey_id;
    db_.write_answer(answer_id, respondent_id, answer_json.dump());

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
