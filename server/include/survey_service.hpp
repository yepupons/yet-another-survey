#ifndef SURVEY_SERVICE_HPP_
#define SURVEY_SERVICE_HPP_

#include <string>
#include <drogon/MultiPart.h>
#include <nlohmann/json.hpp>
#include "database.hpp"

namespace survey {
class SurveyService {
public:
    explicit SurveyService(Database &db);

    std::string create_survey(
        const std::string &access_token,
        const nlohmann::json &payload
    );

    std::string submit_answer(
        const std::string &access_token,
        const nlohmann::json &payload,
        const std::string &survey_id
    );

    std::string check_answer(
        const std::string &access_token,
        const nlohmann::json &payload
    );

    std::string get_statistics(
        const std::string &access_token,
        const std::string &survey_id,
        const std::string &format
    );

    std::string upload_image(
        const std::string &access_token,
        const drogon::HttpFile &file
    );

private:
    Database &db_;

    std::string require_user(const std::string &access_token) const;
    void validate_question_payload(
        const std::string &survey_type,
        const nlohmann::json &question
    ) const; 
    void validate_survey_payload(const nlohmann::json &payload) const;
    void validate_answer_payload(
        const nlohmann::json &survey,
        const nlohmann::json &answer
    ) const;
    void validate_image(const drogon::HttpFile &file) const;
    void require_survey_owner(
        const std::string &survey_id,
        const std::string &user_id
    ) const;
};
}

#endif
