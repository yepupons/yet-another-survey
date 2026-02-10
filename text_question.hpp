#ifndef TEXT_QUESTION_HPP_
#define TEXT_QUESTION_HPP_
#include "abstract_question.hpp"
#include <string>

namespace survey {
class TextBlock : QuestionBlock {
    std::string question_;
    std::string answer_;

public:
    void print() const override;
    bool parse_input(const std::string &) override;
    void save_result(nlohmann::json&) const override;
};
}

#endif // TEXT_QUESTION_HPP_