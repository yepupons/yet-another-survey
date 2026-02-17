#ifndef MULTIPLE_QUESTION_HPP_
#define MULTIPLE_QUESTION_HPP_
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "abstract_question.hpp"

namespace survey {
class MultipleQuestionBlock : public QuestionBlock {
    std::string question_;
    std::vector<std::string> options_;
    std::vector<int> answer_;
    std::vector<int> correct_answer_ = {};

public:
    void print() const override;
    bool parse_input(const std::string &) override;
    void save_result(nlohmann::json &) const override;

    MultipleQuestionBlock(nlohmann::json &block)
        : question_(block["text"]),
          options_(block["options"]) {
    }

    MultipleQuestionBlock(nlohmann::json &block, std::vector<int> correct_answer)
        : question_(block["text"]),
          options_(block["options"]),
          correct_answer_(correct_answer) {
    }
};
}  // namespace survey

#endif  // MULTIPLE_QUESTION_HPP_
