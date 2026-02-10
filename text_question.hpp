#ifndef TEXT_QUESTION_HPP_
#define TEXT_QUESTION_HPP_
#include <string>
#include "abstract_question.hpp"
#include <nlohmann/json_fwd.hpp>

namespace survey {
class TextBlock : public QuestionBlock {
    std::string question_;
    std::string answer_;

public:
    void print() const override;
    bool parse_input(const std::string &) override;
    void save_result(nlohmann::json &) const override;

    TextBlock(nlohmann::json &block) : question_(block["question_text"]) {
    }
};
}  // namespace survey

#endif  // TEXT_QUESTION_HPP_