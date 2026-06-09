#include <QtTest/QtTest>
#include <exception>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
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
            {"sections", nlohmann::json::array({
                {
                    {"title", "First section"},
                    {"questions", nlohmann::json::array({
                        {
                            {"type", "text"},
                            {"text", "How old are you?"},
                            {"required", true}
                        },
                        {
                            {"type", "single"},
                            {"text", "Choose one"},
                            {"required", true},
                            {"options", nlohmann::json::array({"A", "B"})}
                        },
                        {
                            {"type", "multiple"},
                            {"text", "Choose many"},
                            {"required", false},
                            {"options", nlohmann::json::array({"A", "B", "C"})}
                        }
                    })}
                }
            })}
        };
    }

    static nlohmann::json valid_test_survey() {
        auto survey = valid_survey();
        survey["data"]["type"] = "test";
        survey["sections"][0]["questions"][0]["answer"] =
            nlohmann::json::array({"18", "eighteen"});
        survey["sections"][0]["questions"][1]["answer"] = 1;
        survey["sections"][0]["questions"][2]["answer"] =
            nlohmann::json::array({1, 3});
        return survey;
    }

    static nlohmann::json valid_quiz_survey() {
        return {
            {"data", {{"type", "quiz"}}},
            {"title", "Valid quiz"},
            {"sections", nlohmann::json::array({
                {
                    {"questions", nlohmann::json::array({
                        {
                            {"type", "single"},
                            {"text", "Choose one"},
                            {"required", true},
                            {"options", nlohmann::json::array({"A", "B"})},
                            {"scores", nlohmann::json::array({"alpha", "beta"})}
                        }
                    })}
                }
            })}
        };
    }

    static nlohmann::json valid_answer() {
        return {
            {"sections", nlohmann::json::array({
                nlohmann::json::array({
                    {{"answer", "18"}},
                    {{"answer", 1}},
                    {{"answer", nlohmann::json::array({1, 3})}}
                })
            })}
        };
    }

private slots:

    void accepts_valid_survey() {
        survey::SurveyValidator validator;

        QVERIFY_NOTHROW(validator.validate_survey_payload(valid_survey()));
    }

    void accepts_valid_test_survey() {
        survey::SurveyValidator validator;

        QVERIFY_NOTHROW(validator.validate_survey_payload(valid_test_survey()));
    }

    void accepts_valid_quiz_survey() {
        survey::SurveyValidator validator;

        QVERIFY_NOTHROW(validator.validate_survey_payload(valid_quiz_survey()));
    }

    void rejects_missing_data() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload.erase("data");

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_non_object_data() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["data"] = "bad";

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_missing_survey_type() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["data"].erase("type");

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_unknown_survey_type() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["data"]["type"] = "unknown";

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_missing_title() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload.erase("title");

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_long_title() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["title"] = std::string(201, 'a');

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_missing_sections() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload.erase("sections");

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_empty_sections() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["sections"] = nlohmann::json::array();

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_too_many_sections() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["sections"] = nlohmann::json::array();
        for (int i = 0; i < 51; ++i) {
            payload["sections"].push_back(valid_survey()["sections"][0]);
        }

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_section_without_questions() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["sections"][0].erase("questions");

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_empty_questions() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["sections"][0]["questions"] = nlohmann::json::array();

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_too_many_questions() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        const auto question = payload["sections"][0]["questions"][0];
        payload["sections"][0]["questions"] = nlohmann::json::array();
        for (int i = 0; i < 101; ++i) {
            payload["sections"][0]["questions"].push_back(question);
        }

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_question_without_type() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["sections"][0]["questions"][0].erase("type");

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_unknown_question_type() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["sections"][0]["questions"][0]["type"] = "unknown";

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_empty_title() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["title"] = "";

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_question_without_text() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["sections"][0]["questions"][0].erase("text");

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_long_question_text() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["sections"][0]["questions"][0]["text"] = std::string(3001, 'a');

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_non_boolean_required() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["sections"][0]["questions"][0]["required"] = "yes";

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_non_string_image() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["sections"][0]["questions"][0]["image"] = 10;

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_choice_question_without_options() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["sections"][0]["questions"][1].erase("options");

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_empty_options() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["sections"][0]["questions"][1]["options"] =
            nlohmann::json::array();

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_too_many_options() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["sections"][0]["questions"][1]["options"] =
            nlohmann::json::array();
        for (int i = 0; i < 51; ++i) {
            payload["sections"][0]["questions"][1]["options"].push_back("A");
        }

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_non_string_option() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["sections"][0]["questions"][1]["options"][0] = 42;

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_long_option() {
        survey::SurveyValidator validator;
        auto payload = valid_survey();
        payload["sections"][0]["questions"][1]["options"][0] =
            std::string(301, 'a');

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_test_question_without_answer() {
        survey::SurveyValidator validator;
        auto payload = valid_test_survey();
        payload["sections"][0]["questions"][1].erase("answer");

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_quiz_question_without_scores() {
        survey::SurveyValidator validator;
        auto payload = valid_quiz_survey();
        payload["sections"][0]["questions"][0].erase("scores");

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void rejects_quiz_question_with_non_array_scores() {
        survey::SurveyValidator validator;
        auto payload = valid_quiz_survey();
        payload["sections"][0]["questions"][0]["scores"] = "alpha";

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_survey_payload(payload)
        );
    }

    void accepts_valid_answer() {
        survey::SurveyValidator validator;

        QVERIFY_NOTHROW(
            validator.validate_answer_payload(valid_survey(), valid_answer())
        );
    }

    void accepts_optional_empty_single_answer() {
        survey::SurveyValidator validator;
        auto survey = valid_survey();
        auto answer = valid_answer();
        survey["sections"][0]["questions"][1]["required"] = false;
        answer["sections"][0][1]["answer"] = -1;

        QVERIFY_NOTHROW(validator.validate_answer_payload(survey, answer));
    }

    void accepts_optional_empty_multiple_answer() {
        survey::SurveyValidator validator;
        auto survey = valid_survey();
        auto answer = valid_answer();
        survey["sections"][0]["questions"][2]["required"] = false;
        answer["sections"][0][2]["answer"] = nlohmann::json::array();

        QVERIFY_NOTHROW(validator.validate_answer_payload(survey, answer));
    }

    void rejects_answer_without_sections() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer.erase("sections");

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }

    void rejects_answer_section_count_mismatch() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer["sections"].push_back(nlohmann::json::array());

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }

    void rejects_non_array_answer_section() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer["sections"][0] = "bad";

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }

    void rejects_answer_count_mismatch() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer["sections"][0].erase(2);

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }

    void rejects_answer_item_without_answer_field() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer["sections"][0][0].erase("answer");

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }

    void rejects_text_answer_with_wrong_type() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer["sections"][0][0]["answer"] = 18;

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }

    void rejects_required_empty_text_answer() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer["sections"][0][0]["answer"] = "";

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }

    void rejects_long_text_answer() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer["sections"][0][0]["answer"] = std::string(5001, 'a');

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }

    void rejects_single_answer_with_wrong_type() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer["sections"][0][1]["answer"] = "A";

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }

    void rejects_single_answer_out_of_range() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer["sections"][0][1]["answer"] = 3;

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }

    void rejects_required_empty_single_answer() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer["sections"][0][1]["answer"] = -1;

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }

    void rejects_multiple_answer_with_wrong_type() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer["sections"][0][2]["answer"] = 1;

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }

    void rejects_required_empty_multiple_answer() {
        survey::SurveyValidator validator;
        auto survey = valid_survey();
        auto answer = valid_answer();
        survey["sections"][0]["questions"][2]["required"] = true;
        answer["sections"][0][2]["answer"] = nlohmann::json::array();

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(survey, answer)
        );
    }

    void rejects_multiple_answer_out_of_range() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer["sections"][0][2]["answer"] = nlohmann::json::array({1, 4});

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }

    void rejects_duplicate_multiple_answer() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer["sections"][0][2]["answer"] = nlohmann::json::array({1, 1});

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }

    void rejects_non_integer_multiple_answer_option() {
        survey::SurveyValidator validator;
        auto answer = valid_answer();
        answer["sections"][0][2]["answer"] =
            nlohmann::json::array({1, "bad"});

        QVERIFY_THROWS_EXCEPTION(
            std::invalid_argument,
            validator.validate_answer_payload(valid_survey(), answer)
        );
    }
};

QTEST_MAIN(SurveyValidatorTests)
#include "survey_validator_tests.moc"
