#include "text_question.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

namespace survey {
void TextBlock::print() const {
    std::cout << '\n';
    for (int i = 0; i < 10; i++) {std::cout << '-';}
    std::cout << '\n';
    std::cout << question_ << '\n';
    if (!answer_.empty()) {
        if (answer_ == correct_answer_){
            std::cout << "Your answer: \"" << answer_ << "\" is correct!" <<'\n';
        } else {
            std::cout << "Your answer: \"" << answer_ << "\" is NOT correct!" <<'\n';
            std::cout << "Correct answer: " << correct_answer_ << '\n';
        }
        
    } else {
        std::cout << "Input your answer here: ";
    }
}

bool TextBlock::parse_input(const std::string &input) {
    if (!input.empty()) {
        answer_ = input;
        return true;
    }
    std::cout << "Incorrect input!\n";
    return false;
}

void TextBlock::save_result(nlohmann::json &answers_data) const {
    answers_data["answers"].push_back(answer_);
}
}  // namespace survey
