#ifndef SINGLE_CHOICE_QUESTION_HPP_
#define SINGLE_CHOICE_QUESTION_HPP_
#include <string>
#include <vector>
#include "abstract_question.hpp"
#include <nlohmann/json.hpp>


namespace survey {
class SingleChoiceBlock : public QuestionBlock {
    std::string question_;
    std::vector<std::string> options_;
    int answer_ = 0;
    int correct_answer_ = 0;

public:
    void print() const override;
    bool parse_input(const std::string &) override;
    void save_result(nlohmann::json &) const override;

    SingleChoiceBlock(nlohmann::json &block)
        : question_(block["text"]),
          options_(block["options"]) {
    }

    SingleChoiceBlock(nlohmann::json &block, int correct_answer)
        : question_(block["text"]),
          options_(block["options"]),
          correct_answer_(correct_answer) {
    }
};
}  // namespace survey

#endif  // SINGLE_CHOICE_QUESTION_HPP_
