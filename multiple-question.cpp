#include "multiple-question.hpp"
#include <cctype>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

namespace survey {

void MultipleQuestionBlock::print() const {
    std::cout << question_ << "\nChoose one or more answers\n";
    for (int i = 0; i < options_.size(); ++i) {
        std::cout << i + 1 << ". " << options_[i] << '\n';
    }
}

bool MultipleQuestionBlock::parse_input(const std::string &answer) {
    int n = answers_.size();
    for (int i = 0; i < n;) {
        if (isdigit(answer[i])) {
            int x = 0;
            while (i < n && isdigit(answer[i])) {
                x = x * 10 + (answer[i] - '0');
                ++i;
            }
            if (x <= options_.size() && x > 0) {
                answers_.push_back(x);
            }
        } else {
            if (answer[i] == ',' || answer[i] == ' ') {
                ++i;
            } else {
                std::cout << "Incorrect input!\n";
                return false;
            }
        }
    }
    return true;
}

void MultipleQuestionBlock::save_result(nlohmann::json &j) const {
    j["answers"].push_back(answers_);
    std::cout << "Your answer has been saved\n";
}
}  // namespace survey