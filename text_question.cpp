#include "text_question.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

namespace survey {
void TextBlock::print() const {
    std::cout << question_ << '\n';
    std::cout << "Input your answer here: ";
}

bool TextBlock::parse_input(const std::string &input) {
    if (!input.empty()) {
        answer_ = input;
        return true;
    }
    return false;
}

void TextBlock::save_result(nlohmann::json &answers_data) const {
    answers_data["answers"].push_back(answer_);
}
}  // namespace survey