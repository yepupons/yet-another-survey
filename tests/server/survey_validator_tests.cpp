#include <QtTest/QtTest>
#include <exception>
#include <nlohmann/json.hpp>
#include "survey_validator.hpp"

#define QVERIFY_NOTHROW(statement)                      \
    do {                                                \
        try {                                           \
            statement;                                  \
        } catch (const std::exception &e) {             \
            QFAIL(e.what());                            \
        } catch (...) {                                 \
            QFAIL("Unexpected non-standard exception"); \
        }                                               \
    } while (false)

class SurveyValidatorTests : public QObject {
    Q_OBJECT

private:
    static nlohmann::json valid_survey() {
        return {
            {"data", {{"type", "survey"}}},
            {"title", "Valid survey"},
            {"sections",
             nlohmann::json::array(
                 {{{"questions",
                    nlohmann::json::array(
                        {{{"type", "single"},
                          {"text", "Choose one"},
                          {"required", true},
                          {"options", nlohmann::json::array({"A", "B"})}}}
                    )}}}
             )}};
    }

    static nlohmann::json valid_answer() {
        return {
            {"sections",
             nlohmann::json::array({nlohmann::json::array({{{"answer", 1}}})}
             )}};
    }

private slots:

    void accepts_valid_survey() {
        survey::SurveyValidator validator;

        QVERIFY_NOTHROW(validator.validate_survey_payload(valid_survey()));
    }

    void rejects_empty_title() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["title"] = "";

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument, validator.validate_survey_payload(payload)
        );
    }

    void rejects_unknown_survey_type() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["data"]["type"] = "unknown";

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument, validator.validate_survey_payload(payload)
        );
    }

    void accepts_valid_answer() {
        survey::SurveyValidator validator;

        QVERIFY_NOTHROW(
            validator.validate_answer_payload(valid_survey(), valid_answer())
        );
    }

    void rejects_single_answer_out_of_range() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer["sections"][0][0]["answer"] = 3;

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }

    void rejects_duplicate_multiple_answer() {
        survey::SurveyValidator validator;

        auto survey = valid_survey();
        survey["sections"][0]["questions"][0]["type"] = "multiple";

        nlohmann::json answer = {
            {"sections", nlohmann::json::array({nlohmann::json::array(
                             {{{"answer", nlohmann::json::array({1, 1})}}}
                         )})}};

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(survey, answer)
        );
    }
};

QTEST_MAIN(SurveyValidatorTests)
#include "survey_validator_tests.moc"
