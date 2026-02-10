#ifndef SINGLE_CHOICE_QUESTION_HPP_
#define SINGLE_CHOICE_QUESTION_HPP_
#include <string>
#include <vector>
#include "abstract_question.hpp"

namespace survey {
class SingleChoiceBlock : QuestionBlock {
    std::string question_;
    std::vector<std::string> options_;
    int answer_;

public:
    void print() const override;
    bool parse_input(const std::string &) override;
    void save_result(nlohmann::json &) const override;
};
}  // namespace survey

#endif  // SINGLE_CHOICE_QUESTION_HPP_
