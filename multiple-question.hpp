#ifndef MULTIPLE_QUESTION_HPP_
#define MULTIPLE_QUESTION_HPP_
#include <string>
#include <vector>
#include "abstract_question.hpp"

namespace survey {
class MultipleQuestionBlock : QuestionBlock {
public:
    std::string question_text_;
    std::vector<std::string> questions_;
    std::vector<int> answers_;

    void print() const override;
    bool parse_input(const std::string &) override;
    void save_result(nlohmann::json &) const override;

    MultipleQuestionBlock() = default;
    MultipleQuestionBlock(std::string, std::vector<std::string>);
    virtual ~MultipleQuestionBlock() = default;
    MultipleQuestionBlock(const MultipleQuestionBlock &) = delete;
    MultipleQuestionBlock(MultipleQuestionBlock &&) = delete;
    MultipleQuestionBlock &operator=(const MultipleQuestionBlock &) = delete;
    MultipleQuestionBlock &operator=(MultipleQuestionBlock &&) = delete;
};
}  // namespace survey

#endif  // MULTIPLE_QUESTION_HPP_