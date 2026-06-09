#ifndef SURVEY_VALIDATOR_HPP_
#define SURVEY_VALIDATOR_HPP_

#include <string>
#include <nlohmann/json.hpp>

namespace survey {
class SurveyValidator {
public:
    void validate_survey_payload(const nlohmann::json &payload) const;
    void validate_answer_payload(
        const nlohmann::json &survey,
        const nlohmann::json &answer
    ) const;

private:
    void validate_question_payload(
        const std::string &survey_type,
        const nlohmann::json &question
    ) const;
};
}  // namespace survey

#endif  // SURVEY_VALIDATOR_HPP_
