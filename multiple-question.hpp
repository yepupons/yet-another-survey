#ifndef MULTIPLE_QUESTION_HPP_
#define MULTIPLE_QUESTION_HPP_
#include <string>
#include <vector>
#include "abstract_question.hpp"

namespace survey {
class MultipleQuestionBlock : QuestionBlock {
    std::string question_;
    std::vector<std::string> options_;
    std::vector<int> answers_;

public:
    void print() const override;
    bool parse_input(const std::string &) override;
    void save_result(nlohmann::json &) const override;

    MultipleQuestionBlock(const nlohmann::json &block)
        : question_(block["question_block"]),
          options_(block["question_options"]) {
    }
};
}  // namespace survey

#endif  // MULTIPLE_QUESTION_HPP_