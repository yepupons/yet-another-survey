#include "multiple-question.hpp"
#include <cctype>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

namespace survey {

void MultipleQuestionBlock::print() const {
    std::cout << '\n';
    for (int i = 0; i < 10; i++) {std::cout << '-';}
    std::cout << '\n';
    std::cout << question_ << "\nChoose one or more answers\n";
    for (int i = 0, j = 0; i < options_.size(); ++i) {
        std::cout << i + 1 << ". " << options_[i] << '\n';
    }
    if (answer_.empty()) {
        std::cout << "Input your choice here: ";
    } else {
        std::cout << "your answer: ";
        for (int ans : answer_) {
            std::cout << ans << ' ';
        }
        std::cout << '\n';
    }
}

bool MultipleQuestionBlock::parse_input(const std::string &answer) {
    int n = answer.size();
    for (int i = 0; i < n;) {
        if (isdigit(answer[i])) {
            int x = 0;
            while (i < n && isdigit(answer[i])) {
                x = x * 10 + (answer[i] - '0');
                ++i;
            }
            // if (x <= options_.size() && x > 0) {
            answer_.push_back(x);
            // }
        } else {
            if (answer[i] == ',' || answer[i] == ' ') {
                ++i;
            } else {
                std::cout << "Incorrect input!\n";
                return false;
            }
        }
    }
    std::sort(answer_.begin(), answer_.end());
    return true;
}

void MultipleQuestionBlock::save_result(nlohmann::json &answers_data) const {
    answers_data["answers"].push_back(answer_);
}
}  // namespace survey