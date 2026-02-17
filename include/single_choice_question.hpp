#ifndef SINGLE_CHOICE_QUESTION_HPP_
#define SINGLE_CHOICE_QUESTION_HPP_
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>
#include "abstract_question.hpp"

namespace survey {
class SingleChoiceBlock : public QuestionBlock {
    std::string question_;
    std::vector<std::string> options_;
    int answer_ = 0;
    std::optional<int> correct_answer_;

public:
    void print_question() const override;
    bool parse_input(const std::string &) override;
    void save_result(nlohmann::json &) const override;
    void print_answer(const bool) const override;

    SingleChoiceBlock(nlohmann::json &block, std::optional<int> correct_answer)
        : question_(block["text"]),
          options_(block["options"]),
          correct_answer_(correct_answer) {
    }
};
}  // namespace survey

#endif  // SINGLE_CHOICE_QUESTION_HPP_
