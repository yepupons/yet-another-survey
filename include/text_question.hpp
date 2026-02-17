#ifndef TEXT_QUESTION_HPP_
#define TEXT_QUESTION_HPP_
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include "abstract_question.hpp"

namespace survey {
class TextBlock : public QuestionBlock {
    std::string question_;
    std::string answer_;
    std::optional<std::string> correct_answer_;

public:
    void print_question() const override;
    bool parse_input(const std::string &) override;
    void save_result(nlohmann::json &) const override;
    void print_answer(const bool) const override;

    TextBlock(nlohmann::json &block, std::optional<std::string> correct_answer)
        : question_(block["text"]), correct_answer_(correct_answer) {
    }
};
}  // namespace survey

#endif  // TEXT_QUESTION_HPP_
