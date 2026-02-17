#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include "abstract_question.hpp"
#include "multiple_question.hpp"
#include "single_choice_question.hpp"
#include "text_question.hpp"

namespace survey {
enum class BlockType { Text, Multiple, Single };

const std::unordered_map<std::string, BlockType> COMPARATOR{
    {"text", BlockType::Text},
    {"multiple", BlockType::Multiple},
    {"single", BlockType::Single}};
}  // namespace survey

static size_t
WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

int main() {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8080/file");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    nlohmann::json survey_data = nlohmann::json::parse(readBuffer);
    // std::ifstream f("./survey_templates/template_test.json");

    std::vector<std::unique_ptr<survey::QuestionBlock>> block_list;
    const bool is_test =
        survey_data.at("survey_data").value("type", "survey") == "test";

    for (auto &question_block : survey_data.at("questions")) {
        const std::string question_type =
            question_block.at("type").get<std::string>();
        switch (survey::COMPARATOR.at(question_type)) {
            case survey::BlockType::Text: {
                block_list.push_back(std::make_unique<survey::TextBlock>(
                    question_block,
                    (is_test) ? std::make_optional(question_block.at("answer"))
                              : std::nullopt
                ));
                break;
            }
            case survey::BlockType::Multiple: {
                block_list.push_back(
                    std::make_unique<survey::MultipleChoiceBlock>(
                        question_block,
                        (is_test)
                            ? std::make_optional(question_block.at("answer")
                                                     .get<std::vector<int>>())
                            : std::nullopt
                    )
                );
                break;
            }
            case survey::BlockType::Single: {
                block_list.push_back(
                    std::make_unique<survey::SingleChoiceBlock>(
                        question_block,
                        (is_test) ? std::make_optional(
                                        question_block.at("answer").get<int>()
                                    )
                                  : std::nullopt
                    )
                );
                break;
            }
        }
    }

    nlohmann::json answer_data = {
        {"answer_data", {{"survey_id", 0}, {"answer_id", 0}}}, {"answers", {}}};
    answer_data["answer_data"]["survey_id"] = survey_data["survey_id"];
    answer_data["answer_data"]["answer_id"] = 67;

    for (const auto &block : block_list) {
        block->print_question();
        std::string input;
        std::getline(std::cin, input);
        while (!block->parse_input(input)) {
            std::getline(std::cin, input);
        }
        block->save_result(answer_data);
    }

    std::cout << "\n\n\nSurvey is completed! Check your answers:\n";
    for (const auto &block : block_list) {
        block->print_answer(is_test);
    }
    std::ofstream o("test_answer.json");
    o << std::setw(4) << answer_data << std::endl;
}
