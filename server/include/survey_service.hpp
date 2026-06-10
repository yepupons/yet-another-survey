#ifndef SURVEY_SERVICE_HPP_
#define SURVEY_SERVICE_HPP_

#include <string>
#include <drogon/MultiPart.h>
#include <nlohmann/json.hpp>
#include "database.hpp"
#include "survey_validator.hpp"

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

private:
    Database &db_;
    SurveyValidator validator_;
};
}

#endif
