#ifndef MULTIPLE_QUESTION_HPP_
#define MULTIPLE_QUESTION_HPP_
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>
#include "abstract_question.hpp"

namespace survey {
class MultipleChoiceBlock : public QuestionBlock {
    std::string question_;
    std::vector<std::string> options_;
    std::vector<int> answer_;
    std::optional<std::vector<int>> correct_answer_;

public:
    void print_question() const override;
    bool parse_input(const std::string &) override;
    void save_result(nlohmann::json &) const override;
    void print_answer(const bool) const override;

    MultipleChoiceBlock(
        nlohmann::json &block,
        std::optional<std::vector<int>> correct_answer
    )
        : question_(block["text"]),
          options_(block["options"]),
          correct_answer_(correct_answer) {
    }
};
}  // namespace survey

#endif  // MULTIPLE_QUESTION_HPP_
